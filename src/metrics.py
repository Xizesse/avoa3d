#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
import os
import time
from datetime import datetime
import csv
import math

# Plotting imports (optional - only imported when needed)
try:
    import matplotlib
    matplotlib.use('Agg')  # Use non-interactive backend
    import matplotlib.pyplot as plt
    import numpy as np
    PLOTTING_AVAILABLE = True
except ImportError:
    PLOTTING_AVAILABLE = False

class BasicAVOAMetrics(Node):
    def __init__(self):
        super().__init__('basic_avoa_metrics')
        
        # Declare parameters
        self.declare_parameter('scenario', 'default')
        self.declare_parameter('results_directory', 'results')
        self.declare_parameter('auto_generate_plots', True)
        
        # Topic parameters (from your yaml structure)
        self.declare_parameter('topics.desired_vel', '/model/agente/desired_vel')
        self.declare_parameter('topics.cmd_vel', '/model/agente/cmd_vel')
        self.declare_parameter('topics.odometry', '/model/agente/odometry')
        self.declare_parameter('topics.goal_odometry', '/model/goal/odometry')
        
        # Get parameters
        self.scenario = self.get_parameter('scenario').value
        self.results_dir = self.get_parameter('results_directory').value
        
        # Get topic names
        self.desired_vel_topic = self.get_parameter('topics.desired_vel').value
        self.cmd_vel_topic = self.get_parameter('topics.cmd_vel').value
        self.odometry_topic = self.get_parameter('topics.odometry').value
        self.goal_odometry_topic = self.get_parameter('topics.goal_odometry').value
        
        # Create timestamped results directory
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.scenario_dir = os.path.join(self.results_dir, f"{self.scenario}_{timestamp}")
        os.makedirs(self.scenario_dir, exist_ok=True)
        
        # Initialize data storage
        self.data_points = []
        self.start_time = time.time()
        
        # Current message storage - robot
        self.current_odometry = None
        self.current_goal_odometry = None
        self.current_cmd_vel = None
        self.current_desired_vel = None
        
        # Current obstacle odometries (up to 10 obstacles)
        self.obstacle_odometries = {}  # Dictionary: {obstacle_id: odometry_msg}
        
        # Data received flags
        self.received_odometry = False
        self.received_goal = False
        self.received_cmd_vel = False
        self.received_desired_vel = False
        
        # Create robot subscribers
        self.odometry_sub = self.create_subscription(
            Odometry, self.odometry_topic, self.odometry_callback, 10)
        
        self.goal_odometry_sub = self.create_subscription(
            Odometry, self.goal_odometry_topic, self.goal_odometry_callback, 10)
        
        self.cmd_vel_sub = self.create_subscription(
            Twist, self.cmd_vel_topic, self.cmd_vel_callback, 10)
        
        self.desired_vel_sub = self.create_subscription(
            Twist, self.desired_vel_topic, self.desired_vel_callback, 10)
        
        # Create obstacle subscribers (0-9 + main obstacle)
        self.obstacle_subscribers = {}
        
        # Subscribe to main obstacle topic
        self.obstacle_subscribers['main'] = self.create_subscription(
            Odometry, '/model/obstacle/odometry', 
            lambda msg: self.obstacle_callback('main', msg), 10)
        
        # Subscribe to obstacle_0 through obstacle_9
        for i in range(10):
            topic_name = f'/model/obstacle_{i}/odometry'
            self.obstacle_subscribers[i] = self.create_subscription(
                Odometry, topic_name,
                lambda msg, obstacle_id=i: self.obstacle_callback(obstacle_id, msg), 10)
        
        # Create timer for periodic data logging (10 Hz)
        self.timer = self.create_timer(0.1, self.log_data)
        
        # Initialize CSV file
        self.csv_filename = os.path.join(self.scenario_dir, 'metrics_data.csv')
        self.init_csv_file()
        
        self.get_logger().info(f"🎬 AVOA Metrics started for scenario: {self.scenario}")
        self.get_logger().info(f"📁 Results directory: {self.scenario_dir}")
        self.get_logger().info(f"📊 Subscribed to topics:")
        self.get_logger().info(f"  - Robot Odometry: {self.odometry_topic}")
        self.get_logger().info(f"  - Goal: {self.goal_odometry_topic}")
        self.get_logger().info(f"  - Cmd Vel: {self.cmd_vel_topic}")
        self.get_logger().info(f"  - Desired Vel: {self.desired_vel_topic}")
        self.get_logger().info(f"  - Obstacles: /model/obstacle/odometry + /model/obstacle_0-9/odometry")

    def init_csv_file(self):
        """Initialize CSV file with headers"""
        headers = [
            'timestamp', 'time_elapsed',
            'pos_x', 'pos_y', 'pos_z',
            'goal_x', 'goal_y', 'goal_z',
            'cmd_vel_x', 'cmd_vel_y', 'cmd_vel_z',
            'desired_vel_x', 'desired_vel_y', 'desired_vel_z',
            'actual_vel_x', 'actual_vel_y', 'actual_vel_z',
            'distance_to_goal'
        ]
        
        # Add obstacle position columns
        headers.extend(['obstacle_main_x', 'obstacle_main_y'])
        for i in range(10):
            headers.extend([f'obstacle_{i}_x', f'obstacle_{i}_y'])
        
        with open(self.csv_filename, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(headers)

    def odometry_callback(self, msg):
        """Callback for agent odometry"""
        self.current_odometry = msg
        self.received_odometry = True

    def goal_odometry_callback(self, msg):
        """Callback for goal odometry"""
        self.current_goal_odometry = msg
        self.received_goal = True

    def cmd_vel_callback(self, msg):
        """Callback for commanded velocity"""
        self.current_cmd_vel = msg
        self.received_cmd_vel = True

    def desired_vel_callback(self, msg):
        """Callback for desired velocity"""
        self.current_desired_vel = msg
        self.received_desired_vel = True

    def obstacle_callback(self, obstacle_id, msg):
        """Callback for obstacle odometry"""
        self.obstacle_odometries[obstacle_id] = msg
        
        # Log when we first receive an obstacle
        if obstacle_id not in self.obstacle_odometries or len(self.obstacle_odometries) == 1:
            pos = msg.pose.pose.position
            self.get_logger().info(f"🚧 Obstacle {obstacle_id} detected at ({pos.x:.2f}, {pos.y:.2f})")

    def calculate_distance_to_goal(self):
        """Calculate distance from current position to goal"""
        if not (self.received_odometry and self.received_goal):
            return float('inf')
        
        pos = self.current_odometry.pose.pose.position
        goal = self.current_goal_odometry.pose.pose.position
        
        distance = ((pos.x - goal.x)**2 + (pos.y - goal.y)**2 + (pos.z - goal.z)**2)**0.5
        return distance

    def log_data(self):
        """Log current data point to CSV"""
        # Only log if we have received at least odometry data
        if not self.received_odometry:
            return
        
        current_time = time.time()
        elapsed_time = current_time - self.start_time
        
        # Calculate distance to goal
        distance_to_goal = self.calculate_distance_to_goal()
        
        # Prepare data row - robot data
        pos = self.current_odometry.pose.pose.position
        vel = self.current_odometry.twist.twist.linear
        
        # Goal position (use default if not received)
        if self.received_goal:
            goal_pos = self.current_goal_odometry.pose.pose.position
            goal_x, goal_y, goal_z = goal_pos.x, goal_pos.y, goal_pos.z
        else:
            goal_x, goal_y, goal_z = 10.0, 0.0, 0.0  # Default goal
        
        # Command velocity (use zero if not received)
        if self.received_cmd_vel:
            cmd_vel = self.current_cmd_vel.linear
            cmd_vel_x, cmd_vel_y, cmd_vel_z = cmd_vel.x, cmd_vel.y, cmd_vel.z
        else:
            cmd_vel_x, cmd_vel_y, cmd_vel_z = 0.0, 0.0, 0.0
        
        # Desired velocity (use zero if not received)
        if self.received_desired_vel:
            des_vel = self.current_desired_vel.linear
            des_vel_x, des_vel_y, des_vel_z = des_vel.x, des_vel.y, des_vel.z
        else:
            des_vel_x, des_vel_y, des_vel_z = 0.0, 0.0, 0.0
        
        data_row = [
            current_time, elapsed_time,
            pos.x, pos.y, pos.z,
            goal_x, goal_y, goal_z,
            cmd_vel_x, cmd_vel_y, cmd_vel_z,
            des_vel_x, des_vel_y, des_vel_z,
            vel.x, vel.y, vel.z,
            distance_to_goal
        ]
        
        # Add obstacle positions
        # Main obstacle
        if 'main' in self.obstacle_odometries:
            obs_pos = self.obstacle_odometries['main'].pose.pose.position
            data_row.extend([obs_pos.x, obs_pos.y])
        else:
            data_row.extend([0.0, 0.0])
        
        # Obstacles 0-9
        for i in range(10):
            if i in self.obstacle_odometries:
                obs_pos = self.obstacle_odometries[i].pose.pose.position
                data_row.extend([obs_pos.x, obs_pos.y])
            else:
                data_row.extend([0.0, 0.0])
        
        # Write to CSV
        with open(self.csv_filename, 'a', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(data_row)
        
        # Store in memory for potential plotting
        self.data_points.append(data_row)
        
        # Periodic status print
        if int(elapsed_time) % 5 == 0 and len(self.data_points) % 50 == 0:  # Every 5 seconds
            obstacle_count = len(self.obstacle_odometries)
            self.get_logger().info(
                f"[{elapsed_time:.1f}s] Pos: ({pos.x:.2f}, {pos.y:.2f}) | "
                f"Goal dist: {distance_to_goal:.2f}m | "
                f"Data points: {len(self.data_points)} | "
                f"Obstacles: {obstacle_count}"
            )

    def shutdown_callback(self):
        """Called when node is shutting down"""
        self.get_logger().info(f"🛑 Shutting down. Logged {len(self.data_points)} data points")
        self.get_logger().info(f"📁 Data saved to: {self.csv_filename}")
        
        # Auto-generate plots if enabled and we have data
        if self.get_parameter('auto_generate_plots').value and len(self.data_points) > 0:
            self.get_logger().info("📊 Generating plots...")
            self.generate_plots()

    def generate_plots(self):
        """Generate trajectory and performance plots"""
        if not PLOTTING_AVAILABLE:
            self.get_logger().warn("📊 Could not generate plots: matplotlib not installed")
            return

        try:
            import matplotlib.pyplot as plt
            import numpy as np
            from matplotlib.patches import Circle

            data = np.array(self.data_points)

            time_stamps = data[:, 1]
            pos_x = data[:, 2]
            pos_y = data[:, 3]
            goal_x = data[:, 5]
            goal_y = data[:, 6]

            plt.figure(figsize=(10, 10))
            ax = plt.gca()

            agent_radius = 1.42/2
            obstacle_radius = 0.5
            circle_interval = 2.0
            last_time = -circle_interval

            obstacle_colors = ['orange', 'purple', 'brown', 'pink', 'gray',
                            'olive', 'cyan', 'magenta', 'yellow', 'lime', 'red']

            for i in range(len(time_stamps)):
                t = time_stamps[i]
                if t - last_time >= circle_interval:
                    # === Robot footprint circle ===
                    circle = Circle((pos_x[i], pos_y[i]), radius=agent_radius,
                                    edgecolor='blue', facecolor='blue', alpha=0.3, linewidth=1)
                    ax.add_patch(circle)
                    ax.text(pos_x[i], pos_y[i], f"{int(t)}s", ha='center', va='center',
                            fontsize=16, color='black')

                    # === Main obstacle circle ===
                    if data.shape[1] > 19:
                        obs_main_x = data[i, 18]
                        obs_main_y = data[i, 19]
                        if obs_main_x != 0.0 or obs_main_y != 0.0:
                            circle = Circle((obs_main_x, obs_main_y), radius=obstacle_radius,
                                            edgecolor=obstacle_colors[0], facecolor=obstacle_colors[0],
                                            alpha=0.3, linewidth=1)
                            ax.add_patch(circle)
                            ax.text(obs_main_x, obs_main_y, f"{int(t)}s", ha='center', va='center',
                                    fontsize=16, color='black')

                    # === Obstacles 0–9 ===
                    for j in range(10):
                        col_x = 20 + j * 2
                        col_y = 21 + j * 2
                        if data.shape[1] > col_y:
                            obs_x = data[i, col_x]
                            obs_y = data[i, col_y]
                            if obs_x != 0.0 or obs_y != 0.0:
                                color = obstacle_colors[(j + 1) % len(obstacle_colors)]
                                circle = Circle((obs_x, obs_y), radius=obstacle_radius,
                                                edgecolor=color, facecolor=color,
                                                alpha=0.3, linewidth=1)
                                ax.add_patch(circle)
                                ax.text(obs_x, obs_y, f"{int(t)}s", ha='center', va='center',
                                        fontsize=16, color='black')

                    last_time = t

            # Optional: dashed path line
            plt.plot(pos_x, pos_y, 'b--', linewidth=1, alpha=0.4)

            # Start/Goal/End markers
            plt.scatter(pos_x[0], pos_y[0], color='blue', s=120, marker='s',
                        alpha=0.8, zorder=5, edgecolors='darkblue', linewidth=2)
            plt.scatter(goal_x[0], goal_y[0], color='red', s=150, marker='x',
                        zorder=5, linewidth=3)
            plt.scatter(pos_x[-1], pos_y[-1], color='blue', s=120, marker='s',
                        alpha=0.3, zorder=5, edgecolors='darkblue', linewidth=2)

            plt.xlim(-10, 10)
            plt.ylim(-10, 10)
            plt.grid(True, alpha=0.3)
            plt.xlabel('X Position (m)', fontsize=16, fontweight='bold')
            plt.ylabel('Y Position (m)', fontsize=16, fontweight='bold')
            plt.tick_params(axis='both', which='major', labelsize=14)
            plt.axis('equal')

            plot_filename = os.path.join(self.scenario_dir, 'trajectory_plot.png')
            plt.savefig(plot_filename, dpi=300, bbox_inches='tight')
            plt.close()

            self.generate_stats_file()
            self.get_logger().info(f"📊 Trajectory + labeled footprints saved to: {plot_filename}")

        except Exception as e:
            self.get_logger().error(f"📊 Error generating plots: {str(e)}")



    def generate_stats_file(self):
        """Generate a text file with performance statistics"""
        try:
            stats_filename = os.path.join(self.scenario_dir, 'performance_stats.txt')
            
            if not self.data_points:
                return
            
            data = np.array(self.data_points)
            time_elapsed = data[:, 1]
            pos_x = data[:, 2]
            pos_y = data[:, 3]
            distance_to_goal = data[:, -1]  # Last column before obstacle data
            
            # Calculate statistics
            total_time = time_elapsed[-1]
            final_distance = distance_to_goal[-1] if not np.isinf(distance_to_goal[-1]) else float('inf')
            path_length = np.sum(np.sqrt(np.diff(pos_x)**2 + np.diff(pos_y)**2))
            min_distance_to_goal = np.min(distance_to_goal[~np.isinf(distance_to_goal)]) if np.any(~np.isinf(distance_to_goal)) else float('inf')
            average_speed = path_length / total_time if total_time > 0 else 0
            
            # Start and end positions
            start_pos = f"({pos_x[0]:.2f}, {pos_y[0]:.2f})"
            end_pos = f"({pos_x[-1]:.2f}, {pos_y[-1]:.2f})"
            
            # Write statistics to file
            with open(stats_filename, 'w') as f:
                f.write(f"AVOA Performance Statistics - Scenario {self.scenario}\n")
                f.write("=" * 50 + "\n\n")
                
                f.write("TRAJECTORY SUMMARY:\n")
                f.write(f"Start Position: {start_pos}\n")
                f.write(f"End Position: {end_pos}\n")
                f.write(f"Total Time: {total_time:.2f} seconds\n")
                f.write(f"Path Length: {path_length:.2f} meters\n")
                f.write(f"Average Speed: {average_speed:.2f} m/s\n\n")
                
                f.write("GOAL PERFORMANCE:\n")
                if final_distance != float('inf'):
                    f.write(f"Final Distance to Goal: {final_distance:.2f} meters\n")
                    f.write(f"Goal Reached: {'Yes' if final_distance < 0.5 else 'No'}\n")
                else:
                    f.write("Final Distance to Goal: N/A\n")
                    f.write("Goal Reached: Unknown\n")
                
                if min_distance_to_goal != float('inf'):
                    f.write(f"Minimum Distance to Goal: {min_distance_to_goal:.2f} meters\n")
                else:
                    f.write("Minimum Distance to Goal: N/A\n")
                
                f.write(f"\nOBSTACLE SUMMARY:\n")
                f.write(f"Number of Active Obstacles: {len(self.obstacle_odometries)}\n")
                for obs_id in sorted(self.obstacle_odometries.keys()):
                    f.write(f"  - Obstacle {obs_id}: Active\n")
                
                f.write(f"\nData Points Collected: {len(self.data_points)}\n")
                f.write(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            
            self.get_logger().info(f"📋 Performance stats saved to: {stats_filename}")
            
        except Exception as e:
            self.get_logger().error(f"📋 Error generating stats file: {str(e)}")

def main(args=None):
    rclpy.init(args=args)
    
    metrics_node = BasicAVOAMetrics()
    
    try:
        rclpy.spin(metrics_node)
    except KeyboardInterrupt:
        pass
    finally:
        metrics_node.shutdown_callback()
        metrics_node.destroy_node()
        try:
            rclpy.shutdown()
        except:
            pass  # Ignore shutdown errors

if __name__ == '__main__':
    main()