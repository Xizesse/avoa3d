#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist, Pose, Vector3
from nav_msgs.msg import Odometry
from geometry_msgs.msg import PoseStamped
from std_msgs.msg import Header
from avoa3d.msg import ElementCharacteristicsArray, ElementCharacteristicsStamped
import os
import time
from datetime import datetime
import csv
import math
import numpy as np

# Plotting imports (optional - only imported when needed)
try:
    import matplotlib
    matplotlib.use('Agg')  # Use non-interactive backend
    import matplotlib.pyplot as plt
    PLOTTING_AVAILABLE = True
except ImportError:
    PLOTTING_AVAILABLE = False



class BasicAVOATrackingMetrics(Node):
    def __init__(self):
        super().__init__('basic_avoa_tracking_metrics')
        
        # Declare parameters
        self.declare_parameter('scenario', 'tracking_default')
        self.declare_parameter('results_directory', 'results')
        self.declare_parameter('auto_generate_plots', True)
        
        # Topic parameters
        self.declare_parameter('topics.desired_vel', '/model/agente/desired_vel')
        self.declare_parameter('topics.cmd_vel', '/avoa/cmd_vel')
        self.declare_parameter('topics.odometry', '/nest/odometry')
        self.declare_parameter('topics.goal_pose', 'goal_pose')
        self.declare_parameter('topics.element_tracking', 'element_tracking/elements')
        
        # Get parameters
        self.scenario = self.get_parameter('scenario').value
        self.results_dir = self.get_parameter('results_directory').value
        
        # Get topic names
        self.desired_vel_topic = self.get_parameter('topics.desired_vel').value
        self.cmd_vel_topic = self.get_parameter('topics.cmd_vel').value
        self.odometry_topic = self.get_parameter('topics.odometry').value
        self.goal_pose_topic = self.get_parameter('topics.goal_pose').value
        self.element_tracking_topic = self.get_parameter('topics.element_tracking').value
        
        # Create timestamped results directory
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.scenario_dir = os.path.join(self.results_dir, f"{self.scenario}_{timestamp}")
        os.makedirs(self.scenario_dir, exist_ok=True)
        
        # Initialize data storage
        self.data_points = []
        self.start_time = time.time()
        
        # Current message storage - robot
        self.current_odometry = None
        self.current_cmd_vel = None
        self.current_desired_vel = None
        self.current_elements = []  # List of tracked elements in world coordinates
        
        # Data received flags
        self.received_odometry = False
        self.received_cmd_vel = False
        self.received_desired_vel = False
        self.received_elements = False
        
        # Robot pose for coordinate transformation
        self.robot_pose = None
        
        # Create robot subscribers
        self.odometry_sub = self.create_subscription(
            Odometry, self.odometry_topic, self.odometry_callback, 10)
        
        self.cmd_vel_sub = self.create_subscription(
            Twist, self.cmd_vel_topic, self.cmd_vel_callback, 10)
        
        self.desired_vel_sub = self.create_subscription(
            Twist, self.desired_vel_topic, self.desired_vel_callback, 10)
        
        self.goal_pose_sub = self.create_subscription(
            PoseStamped, self.goal_pose_topic, self.goal_pose_callback, 10)
    
        self.goal_pose_from_topic = None
        
        # Subscribe to element tracking
        from rclpy.qos import QoSProfile, QoSReliabilityPolicy
        qos_profile = QoSProfile(depth=10)
        qos_profile.reliability = QoSReliabilityPolicy.BEST_EFFORT
        
        self.element_tracking_sub = self.create_subscription(
            ElementCharacteristicsArray,
            self.element_tracking_topic, 
            self.element_tracking_callback, 
            qos_profile)
        
        # Create timer for periodic data logging (10 Hz)
        self.timer = self.create_timer(0.1, self.log_data)
        
        # Initialize CSV file
        self.csv_filename = os.path.join(self.scenario_dir, 'metrics.csv')
        self.init_csv_file()
        
        self.get_logger().info(f"🎬 AVOA Tracking Metrics started for scenario: {self.scenario}")
        self.get_logger().info(f"📁 Results directory: {self.scenario_dir}")
        self.get_logger().info(f"📊 Subscribed to topics:")
        self.get_logger().info(f"  - Robot Odometry: {self.odometry_topic}")
        self.get_logger().info(f"  - Goal: {self.goal_pose_topic}")
        self.get_logger().info(f"  - Cmd Vel: {self.cmd_vel_topic}")
        self.get_logger().info(f"  - Desired Vel: {self.desired_vel_topic}")
        self.get_logger().info(f"  - Element Tracking: {self.element_tracking_topic}")

    def init_csv_file(self):
        """Initialize CSV file with headers"""
        headers = [
            'timestamp', 'time_elapsed',
            'pos_x', 'pos_y', 'pos_z',
            'goal_x', 'goal_y', 'goal_z',
            'cmd_vel_x', 'cmd_vel_y', 'cmd_vel_z',
            'desired_vel_x', 'desired_vel_y', 'desired_vel_z',
            'actual_vel_x', 'actual_vel_y', 'actual_vel_z',
            'distance_to_goal',
            'num_tracked_elements',
            'cmd_ang_x', 'cmd_ang_y', 'cmd_ang_z',
            'desired_ang_x', 'desired_ang_y', 'desired_ang_z',
            'actual_ang_x', 'actual_ang_y', 'actual_ang_z'
        ]

        # Add columns for up to 10 tracked elements (id, x, y, z, size)
        for i in range(10):
            headers.extend([f'element_{i}_id', f'element_{i}_x', f'element_{i}_y', 
                        f'element_{i}_z', f'element_{i}_size'])

        with open(self.csv_filename, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow([f"Scenario: {self.scenario}"])
            writer.writerow(headers)


    def odometry_callback(self, msg):
        """Callback for agent odometry"""
        self.current_odometry = msg
        self.robot_pose = msg.pose.pose
        self.received_odometry = True

    def goal_pose_callback(self, msg):
        self.goal_pose_from_topic = msg.pose.position

    def cmd_vel_callback(self, msg):
        """Callback for commanded velocity"""
        self.current_cmd_vel = msg
        self.received_cmd_vel = True

    def desired_vel_callback(self, msg):
        """Callback for desired velocity"""
        self.current_desired_vel = msg
        self.received_desired_vel = True

    def element_tracking_callback(self, msg):
        """Callback for element tracking data"""
        if self.robot_pose is None:
            return  # Can't transform without robot pose
            
        try:
            # Transform elements from robot frame to world frame
            self.current_elements = []
            
            # Access the elements array from the message
            elements = msg.elements  # ElementCharacteristicsArray contains elements field
            
            for element in elements:
                # Get element pose in robot frame
                element_pose_robot = element.pose
                element_size = element.size
                element_id = element.id
                
                # DEBUG: Log raw robot-frame data
                robot_frame_x = element_pose_robot.position.x
                robot_frame_y = element_pose_robot.position.y
                robot_frame_z = element_pose_robot.position.z
                
                # Transform to world frame
                world_pose = self.transform_robot_to_world(element_pose_robot)
                
                # Calculate obstacle radius from size (max of x, y)
                obstacle_radius = max(element_size.x, element_size.y) / 2.0
                
                # Store transformed element data
                self.current_elements.append({
                    'id': element_id,
                    'x': world_pose.position.x,
                    'y': world_pose.position.y,
                    'z': world_pose.position.z,
                    'size': obstacle_radius * 2,  # Store diameter
                    'radius': obstacle_radius,
                    'robot_frame_x': robot_frame_x,  # Store for debugging
                    'robot_frame_y': robot_frame_y,
                    'robot_frame_z': robot_frame_z
                })
            
            self.received_elements = True
            
        except Exception as e:
            self.get_logger().error(f"Error processing element tracking data: {e}")
            import traceback
            self.get_logger().error(traceback.format_exc())

    def transform_robot_to_world(self, pose_robot):
        """Transform pose from robot frame to world frame"""
        # Get robot pose in world frame
        robot_pos = self.robot_pose.position
        robot_ori = self.robot_pose.orientation
        
        # Convert quaternion to rotation matrix (simplified for 2D case)
        # For full 3D transformation, you'd need proper quaternion math
        yaw = self.quaternion_to_yaw(robot_ori)
        
        # Transform position
        cos_yaw = math.cos(yaw)
        sin_yaw = math.sin(yaw)
        
        # Rotate and translate
        world_x = robot_pos.x + (pose_robot.position.x * cos_yaw - pose_robot.position.y * sin_yaw)
        world_y = robot_pos.y + (pose_robot.position.x * sin_yaw + pose_robot.position.y * cos_yaw)
        world_z = robot_pos.z + pose_robot.position.z
        
        # Create world pose
        from geometry_msgs.msg import Pose, Point
        world_pose = Pose()
        world_pose.position = Point(x=world_x, y=world_y, z=world_z)
        world_pose.orientation = robot_ori  # Simplified - should add robot's orientation
        
        return world_pose

    def quaternion_to_yaw(self, quaternion):
        """Convert quaternion to yaw angle"""
        # Extract yaw from quaternion
        siny_cosp = 2 * (quaternion.w * quaternion.z + quaternion.x * quaternion.y)
        cosy_cosp = 1 - 2 * (quaternion.y * quaternion.y + quaternion.z * quaternion.z)
        return math.atan2(siny_cosp, cosy_cosp)

    def calculate_distance_to_goal(self):
        """Calculate distance from current position to goal"""
        if self.current_odometry is None or self.goal_pose_from_topic is None:
            return float('inf')
        
        pos = self.current_odometry.pose.pose.position
        goal = self.goal_pose_from_topic
        
        return ((pos.x - goal.x)**2 + (pos.y - goal.y)**2 + (pos.z - goal.z)**2)**0.5

    def log_data(self):
        """Log current data point to CSV"""
        # Only log if we have received at least odometry data
        if not self.received_odometry:
            return

        current_time = time.time()
        elapsed_time = current_time - self.start_time

        # Core robot state
        pos = self.current_odometry.pose.pose.position
        vel = self.current_odometry.twist.twist.linear
        ang_vel = self.current_odometry.twist.twist.angular

        # Goal position (fallback if not received)
        if self.goal_pose_from_topic:
            goal = self.goal_pose_from_topic
            goal_x, goal_y, goal_z = goal.x, goal.y, goal.z
        else:
            goal_x, goal_y, goal_z = 10.0, 0.0, 0.0

        # Commanded velocity (LINEAR + ANGULAR)
        if self.received_cmd_vel:
            cmd_lin = self.current_cmd_vel.linear
            cmd_ang = self.current_cmd_vel.angular
            cmd_vel_x, cmd_vel_y, cmd_vel_z = cmd_lin.x, cmd_lin.y, cmd_lin.z
            cmd_ang_x, cmd_ang_y, cmd_ang_z = cmd_ang.x, cmd_ang.y, cmd_ang.z
        else:
            cmd_vel_x = cmd_vel_y = cmd_vel_z = 0.0
            cmd_ang_x = cmd_ang_y = cmd_ang_z = 0.0

        # Desired velocity (LINEAR + ANGULAR)
        if self.received_desired_vel:
            des_lin = self.current_desired_vel.linear
            des_ang = self.current_desired_vel.angular
            des_vel_x, des_vel_y, des_vel_z = des_lin.x, des_lin.y, des_lin.z
            des_ang_x, des_ang_y, des_ang_z = des_ang.x, des_ang.y, des_ang.z
        else:
            des_vel_x = des_vel_y = des_vel_z = 0.0
            des_ang_x = des_ang_y = des_ang_z = 0.0

        distance_to_goal = self.calculate_distance_to_goal()

        # Build data row
        data_row = [
            current_time, elapsed_time,
            pos.x, pos.y, pos.z,
            goal_x, goal_y, goal_z,
            cmd_vel_x, cmd_vel_y, cmd_vel_z,
            des_vel_x, des_vel_y, des_vel_z,
            vel.x, vel.y, vel.z,
            distance_to_goal,
            len(self.current_elements),
            cmd_ang_x, cmd_ang_y, cmd_ang_z,
            des_ang_x, des_ang_y, des_ang_z,
            ang_vel.x, ang_vel.y, ang_vel.z
        ]
        
        # Add tracked elements data (up to 10 elements)
        for i in range(10):
            if i < len(self.current_elements):
                element = self.current_elements[i]
                data_row.extend([element['id'], element['x'], element['y'], 
                               element['z'], element['size']])
                
                # DEBUG: Log coordinate comparison for first few data points
                if len(self.data_points) < 5 and 'robot_frame_x' in element:
                    self.get_logger().info(f"🔍 LOG DEBUG - Element {element['id']}:")
                    self.get_logger().info(f"    Robot frame coords: ({element['robot_frame_x']:.3f}, {element['robot_frame_y']:.3f})")
                    self.get_logger().info(f"    World coords (CSV): ({element['x']:.3f}, {element['y']:.3f})")
                    self.get_logger().info(f"    Robot world pos: ({pos.x:.3f}, {pos.y:.3f})")
            else:
                data_row.extend([np.nan, np.nan, np.nan, np.nan, np.nan])

        # Save to CSV
        with open(self.csv_filename, 'a', newline='') as csvfile:
            csv.writer(csvfile).writerow(data_row)

        self.data_points.append(data_row)

        # Periodic console status
        if int(elapsed_time) % 5 == 0 and len(self.data_points) % 50 == 0:
            self.get_logger().info(
                f"[{elapsed_time:.1f}s] Pos: ({pos.x:.2f}, {pos.y:.2f}) | "
                f"Goal dist: {distance_to_goal:.2f} m | "
                f"Data points: {len(self.data_points)} | "
                f"Tracked elements: {len(self.current_elements)}"
            )

    def shutdown_callback(self):
        """Called when node is shutting down"""
        self.get_logger().info(f"🛑 Shutting down. Logged {len(self.data_points)} data points")
        self.get_logger().info(f"📁 Data saved to: {self.csv_filename}")
        
        # Auto-generate plots if enabled and we have data
        if self.get_parameter('auto_generate_plots').value and len(self.data_points) > 0:
            self.get_logger().info("📊 Generating XY trajectory plot...")
            self.generate_xy_plot(show_labels=True)
            self.generate_xy_plot(show_labels=False)
            
            self.get_logger().info("📊 Generating velocity components plot...")
            self.generate_command_velocity_plot()
            self.generate_obstacle_velocity_plot()


    def generate_xy_plot(self, show_labels=True):
        """Generate XY trajectory plot highlighting obstacle detection with sharp outlines."""
        if not PLOTTING_AVAILABLE:
            self.get_logger().warn("📊 Could not generate plots: matplotlib not installed")
            return

        try:
            import matplotlib.pyplot as plt
            import numpy as np
            from matplotlib.patches import Circle

            data = np.array(self.data_points)
            if data.size == 0:
                self.get_logger().warn("📊 No data recorded – skipping XY plot.")
                return

            time_stamps = data[:, 1]
            pos_x = data[:, 2]
            pos_y = data[:, 3]
            goal_x = data[:, 5]
            goal_y = data[:, 6]

            plt.figure(figsize=(10, 10))
            ax = plt.gca()

            agent_radius = 0.71
            circle_interval = 2.0

            # Gather obstacle trajectory points for the first tracked element
            obstacle_x, obstacle_y = [], []
            for i in range(len(data)):
                num_elements = int(data[i, 18]) if not np.isnan(data[i, 18]) else 0
                if num_elements > 0:
                    element_start = 28  # FIXED: elements start at column 28
                    obs_x, obs_y = data[i, element_start + 1], data[i, element_start + 2]
                    if not (np.isnan(obs_x) or np.isnan(obs_y)):
                        obstacle_x.append(obs_x)
                        obstacle_y.append(obs_y)

            # Plot obstacle trajectory line (solid, no fading)
            if len(obstacle_x) > 1:
                plt.plot(obstacle_x, obstacle_y, color='red', linestyle='-', linewidth=2.5, alpha=1.0)

            # Dashed robot trajectory line
            plt.plot(pos_x, pos_y, 'b--', linewidth=1, alpha=0.4)

            # Circles at intervals
            last_time = -circle_interval
            for i in range(len(time_stamps)):
                t = time_stamps[i]
                if t - last_time >= circle_interval:
                    # Robot footprint circle
                    robot_circle = Circle((pos_x[i], pos_y[i]), radius=agent_radius,
                                        edgecolor='blue', facecolor='blue', alpha=0.4, linewidth=1)
                    ax.add_patch(robot_circle)

                    if show_labels:
                        ax.text(pos_x[i], pos_y[i], f"{int(t)}s", fontsize=16, color='black',
                                ha='center', va='center')

                    # Obstacle circle with solid outline and subtle fill
                    num_elements = int(data[i, 18]) if not np.isnan(data[i, 18]) else 0
                    if num_elements > 0:
                        element_start = 28  # FIXED: elements start at column 28
                        obs_x, obs_y = data[i, element_start + 1], data[i, element_start + 2]
                        obs_size = data[i, element_start + 4] if not np.isnan(data[i, element_start + 4]) else 1.0
                        if not (np.isnan(obs_x) or np.isnan(obs_y)):
                            obs_radius = max(obs_size / 2.0, 0.5)
                            red_outline = (1.0, 0.0, 0.0, 1.0)  # opaque red edge
                            red_fill = (1.0, 0.0, 0.0, 0.1)    # very transparent red fill
                            obs_circle = Circle(
                                (obs_x, obs_y), radius=obs_radius,
                                edgecolor=red_outline, facecolor=red_fill, linewidth=1)
                            ax.add_patch(obs_circle)
                            if show_labels:
                                ax.text(obs_x, obs_y, f"{int(t)}s", fontsize=16, color='black',
                                        ha='center', va='center')
                    last_time = t

            # Start, goal, end markers
            plt.scatter(pos_x[0], pos_y[0], color='blue', s=120, marker='s',
                        alpha=0.8, zorder=5, edgecolors='darkblue', linewidth=2)
            plt.scatter(goal_x[0], goal_y[0], color='red', s=150, marker='x',
                        zorder=5, linewidth=3)
            plt.scatter(pos_x[-1], pos_y[-1], color='blue', s=120, marker='s',
                        alpha=0.3, zorder=5, edgecolors='darkblue', linewidth=2)

            plt.xlim(-15, 15)
            plt.ylim(-15, 15)
            plt.grid(True, alpha=0.3)
            plt.xlabel('X Position (m)', fontsize=30, fontweight='bold')
            plt.ylabel('Y Position (m)', fontsize=30, fontweight='bold')
            plt.tick_params(axis='both', which='major', labelsize=30)
            plt.axis('equal')

            suffix = 'with_labels' if show_labels else 'no_labels'
            filename = os.path.join(self.scenario_dir, f'{self.scenario}_xy_trajectory_tracking_{suffix}.png')
            plt.savefig(filename, dpi=300, bbox_inches='tight')
            plt.close()

            self.get_logger().info(f"📊 XY trajectory plot saved: {filename}")

        except Exception as e:
            self.get_logger().error(f"📊 Error generating XY plot: {e}")
            import traceback
            self.get_logger().error(traceback.format_exc())

    def generate_obstacle_velocity_plot(self):
        """Plot obstacle linear velocity magnitude over time"""
        if not PLOTTING_AVAILABLE:
            self.get_logger().warn("📊 Skipping obstacle velocity plot: matplotlib not installed")
            return

        try:
            import matplotlib.pyplot as plt
            import numpy as np
            import os

            data = np.array(self.data_points)
            if data.shape[0] == 0:
                self.get_logger().warn("📊 No data to plot.")
                return

            t = data[:, 1]  # elapsed time
            
            # Extract obstacle positions (first tracked element)
            element_start = 28
            obs_x = data[:, element_start + 1]  # element_0_x
            obs_y = data[:, element_start + 2]  # element_0_y
            obs_z = data[:, element_start + 3]  # element_0_z
            
            # Filter out NaN values and find valid data points
            valid_mask = ~(np.isnan(obs_x) | np.isnan(obs_y) | np.isnan(obs_z))
            
            if np.sum(valid_mask) < 2:
                self.get_logger().warn("📊 Not enough valid obstacle data points for velocity calculation")
                return
                
            # Extract valid data
            t_valid = t[valid_mask]
            x_valid = obs_x[valid_mask]
            y_valid = obs_y[valid_mask]
            z_valid = obs_z[valid_mask]
            
            # Calculate velocity using gradient (derivative)
            dt = np.gradient(t_valid)
            vx = np.gradient(x_valid) / dt
            vy = np.gradient(y_valid) / dt
            vz = np.gradient(z_valid) / dt
            
            # Calculate velocity magnitude
            velocity_magnitude = np.sqrt(vx**2 + vy**2 + vz**2)
            
            # Create plot
            plt.figure(figsize=(10, 6))
            plt.plot(t_valid, velocity_magnitude, '-', color='red', linewidth=2.5, label='Obstacle Speed')
            
            plt.xlabel('Time [s]', fontsize=30, fontweight='bold')
            plt.ylabel('Obstacle Speed [m/s]', fontsize=30, fontweight='bold')
            plt.title('Tracked Obstacle Velocity Magnitude', fontsize=30, fontweight='bold')
            plt.grid(True, alpha=0.3)
            plt.legend(fontsize=30)
            plt.tick_params(axis='both', labelsize=30)
            plt.tight_layout()

            filename = os.path.join(self.scenario_dir, f'{self.scenario}_obstacle_velocity_magnitude.png')
            plt.savefig(filename, dpi=300, bbox_inches='tight')
            plt.close()

            self.get_logger().info(f"📈 Obstacle velocity plot saved: {filename}")
            self.get_logger().info(f"📊 Obstacle max speed: {np.max(velocity_magnitude):.2f} m/s")

        except Exception as e:
            self.get_logger().error(f"📉 Error generating obstacle velocity plot: {e}")
            import traceback
            self.get_logger().error(traceback.format_exc())

    def generate_command_velocity_plot(self):
        """Plot commanded linear vx and angular wz over time."""
        if not PLOTTING_AVAILABLE:
            self.get_logger().warn("📊 Skipping commanded velocity plot: matplotlib not installed")
            return

        try:
            import matplotlib.pyplot as plt
            import numpy as np
            import os

            data = np.array(self.data_points)
            if data.shape[0] == 0:
                self.get_logger().warn("📊 No data to plot.")
                return

            t = data[:, 1]

            # FIXED: Correct column indices for tracking metrics
            # Based on the CSV structure:
            # Column 8: cmd_vel_x (vx)
            # Column 21: cmd_ang_z (wz)
            vx_cmd = data[:, 8]   # cmd_vel_x
            wz_cmd = data[:, 21]  # cmd_ang_z

            fig, axs = plt.subplots(2, 1, figsize=(10, 8), sharex=True)

            axs[0].plot(t, vx_cmd, '-', color='green', linewidth=3.5, label='Cmd vx')
            axs[0].set_ylabel('Cmd vx [m/s]', fontsize=30)
            axs[0].tick_params(axis='both', labelsize=30)
            axs[0].grid(True, alpha=0.3)
            axs[0].legend(fontsize=20)

            axs[1].plot(t, wz_cmd, '-', color='orange', linewidth=3.5, label='Cmd wz')
            axs[1].set_ylabel('Cmd wz [rad/s]', fontsize=30)
            axs[1].tick_params(axis='both', labelsize=30)
            axs[1].grid(True, alpha=0.3)
            axs[1].legend(fontsize=20)

            axs[1].set_xlabel('Time [s]', fontsize=30)
            fig.suptitle('Linear vs Angular Velocity (Diff Drive)', fontsize=30, fontweight='bold')
            fig.tight_layout(rect=[0, 0, 1, 0.95])

            filename = os.path.join(self.scenario_dir, f'{self.scenario}_cmd_velocity_vx_wz.png')
            plt.savefig(filename, dpi=300, bbox_inches='tight')
            plt.close()

            self.get_logger().info(f"📈 Commanded velocity plot saved: {filename}")

        except Exception as e:
            self.get_logger().error(f"📉 Error generating commanded velocity plot: {e}")
            import traceback
            self.get_logger().error(traceback.format_exc())


def main(args=None):
    rclpy.init(args=args)
    
    metrics_node = BasicAVOATrackingMetrics()
    
    try:
        rclpy.spin(metrics_node)
    except KeyboardInterrupt:
        pass
    finally:
        try:
            metrics_node.shutdown_callback()
        finally:
            metrics_node.destroy_node()
            if rclpy.ok():
                rclpy.shutdown()

if __name__ == '__main__':
    main()
