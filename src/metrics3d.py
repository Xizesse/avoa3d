#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from geometry_msgs.msg import PoseStamped
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
        self.declare_parameter('results_directory', os.path.expanduser('~/ros2_ws/src/avoa3d/results'))

        self.declare_parameter('auto_generate_plots', True)
        
        # Topic parameters (from your yaml structure)
        # self.declare_parameter('topics.desired_vel', '/model/agente/desired_vel')
        # self.declare_parameter('topics.cmd_vel', '/model/agente/cmd_vel')
        # self.declare_parameter('topics.odometry', '/model/agente/odometry')
        # self.declare_parameter('topics.goal_pose', 'goal_pose')

        self.declare_parameter('topics.desired_vel', '/model/agente/desired_vel')
        self.declare_parameter('topics.cmd_vel', '/model/agente/cmd_vel')
        self.declare_parameter('topics.odometry', '/model/agente/odometry')
        self.declare_parameter('topics.goal_pose', '/goal_pose')

        # Get parameters
        self.scenario = self.get_parameter('scenario').value
        self.results_dir = self.get_parameter('results_directory').value
        
        # Get topic names
        self.desired_vel_topic = self.get_parameter('topics.desired_vel').value
        self.cmd_vel_topic = self.get_parameter('topics.cmd_vel').value
        self.odometry_topic = self.get_parameter('topics.odometry').value
        self.goal_pose_topic = self.get_parameter('topics.goal_pose').value
        
        
        # Create timestamped results directory
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.scenario_dir = os.path.join(self.results_dir, f"{self.scenario}_{timestamp}")
        os.makedirs(self.scenario_dir, exist_ok=True)
        
        # Initialize data storage
        self.data_points = []
        self.start_time = None
        self.start_time_source = None
        
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
        
        self.cmd_vel_sub = self.create_subscription(
            Twist, self.cmd_vel_topic, self.cmd_vel_callback, 10)
        
        self.desired_vel_sub = self.create_subscription(
            Twist, self.desired_vel_topic, self.desired_vel_callback, 10)
        
        self.goal_pose_sub = self.create_subscription(
            PoseStamped, self.goal_pose_topic, self.goal_pose_callback, 10)
    
        self.goal_pose_from_topic = None
        
        # Create obstacle subscribers (0-9 + main obstacle)
        self.obstacle_subscribers = {}
        
        # Subscribe to main obstacle topic
        self.obstacle_subscribers['main'] = self.create_subscription(
            Odometry, '/model/obstacle/odometry', 
            lambda msg: self.obstacle_callback('main', msg), 10)
        
        # --- Fix lambda subscription bug in Python 3.12 / ROS Jazzy ---
        def make_obstacle_callback(obstacle_id):
            def callback(msg):
                self.obstacle_callback(obstacle_id, msg)
            return callback

        for i in range(10):
            topic_name = f'/model/obstacle_{i}/odometry'
            cb = make_obstacle_callback(i)
            self.obstacle_subscribers[i] = self.create_subscription(
                Odometry, topic_name, cb, 10)

        # Create timer for periodic data logging (10 Hz)
        self.timer = self.create_timer(0.1, self.log_data)
        
        # Initialize CSV file
        self.csv_filename = os.path.join(self.scenario_dir, 'metrics.csv')
        self.init_csv_file()
        
        self.get_logger().info(f"🎬 AVOA Metrics started for scenario: {self.scenario}")
        self.get_logger().info(f"📁 Results directory: {self.scenario_dir}")
        self.get_logger().info(f"📊 Subscribed to topics:")
        self.get_logger().info(f"  - Robot Odometry: {self.odometry_topic}")
        self.get_logger().info(f"  - Goal: {self.goal_pose_topic}")
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
        headers.extend(['obstacle_main_x', 'obstacle_main_y', 'obstacle_main_z'])
        for i in range(10):
            headers.extend([f'obstacle_{i}_x', f'obstacle_{i}_y', f'obstacle_{i}_z'])

        # NEW: Append angular velocity columns (after obstacle data)
        headers.extend([
            'cmd_ang_x', 'cmd_ang_y', 'cmd_ang_z',
            'desired_ang_x', 'desired_ang_y', 'desired_ang_z',
            'actual_ang_x', 'actual_ang_y', 'actual_ang_z'
        ])

        with open(self.csv_filename, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow([f"Scenario: {self.scenario}"])
            writer.writerow(headers)

    def odometry_callback(self, msg):
        """Callback for agent odometry"""
        self.current_odometry = msg
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

    def obstacle_callback(self, obstacle_id, msg):
        """Callback for obstacle odometry"""
        self.obstacle_odometries[obstacle_id] = msg
        
        # Log when we first receive an obstacle
        if obstacle_id not in self.obstacle_odometries or len(self.obstacle_odometries) == 1:
            pos = msg.pose.pose.position
            # self.get_logger().info(f"🚧 Obstacle {obstacle_id} detected at ({pos.x:.2f}, {pos.y:.2f})")

    def calculate_distance_to_goal(self):
        """Calculate distance from current position to goal"""
        if self.current_odometry is None or self.goal_pose_from_topic is None:
            return float('inf')
        
        pos = self.current_odometry.pose.pose.position
        goal = self.goal_pose_from_topic
        
        return ((pos.x - goal.x)**2 + (pos.y - goal.y)**2 + (pos.z - goal.z)**2)**0.5

    def log_data(self):
        """Log current data point to CSV (now with angular velocities)"""
        # Only log if we have received at least odometry data
        if not self.received_odometry:
            return

        now_sim = self.get_clock().now().nanoseconds / 1e9
        if now_sim > 0.0:
            if self.start_time is None or self.start_time_source != "sim":
                self.start_time = now_sim
                self.start_time_source = "sim"
            current_time = now_sim
        else:
            current_time = time.time()
            if self.start_time is None:
                self.start_time = current_time
                self.start_time_source = "wall"
        elapsed_time = current_time - self.start_time

        # ------------------------------------------------------------------
        # 1.  Core robot state
        # ------------------------------------------------------------------
        pos = self.current_odometry.pose.pose.position
        vel = self.current_odometry.twist.twist.linear

        # Goal position (fallback if not received)
        if self.goal_pose_from_topic:
            goal = self.goal_pose_from_topic
            goal_x, goal_y, goal_z = goal.x, goal.y, goal.z
        else:
            goal_x, goal_y, goal_z = 10.0, 0.0, 0.0
        # Commanded linear velocity
        if self.received_cmd_vel:
            cmd_lin      = self.current_cmd_vel.linear
            cmd_ang      = self.current_cmd_vel.angular
            cmd_vel_x, cmd_vel_y, cmd_vel_z = cmd_lin.x, cmd_lin.y, cmd_lin.z
            cmd_ang_x, cmd_ang_y, cmd_ang_z = cmd_ang.x, cmd_ang.y, cmd_ang.z
        else:
            cmd_vel_x = cmd_vel_y = cmd_vel_z = 0.0
            cmd_ang_x = cmd_ang_y = cmd_ang_z = 0.0

        # Desired linear velocity
        if self.received_desired_vel:
            des_lin      = self.current_desired_vel.linear
            des_ang      = self.current_desired_vel.angular
            des_vel_x, des_vel_y, des_vel_z = des_lin.x, des_lin.y, des_lin.z
            des_ang_x, des_ang_y, des_ang_z = des_ang.x, des_ang.y, des_ang.z
        else:
            des_vel_x = des_vel_y = des_vel_z = 0.0
            des_ang_x = des_ang_y = des_ang_z = 0.0

        # Actual angular velocity from odometry
        act_ang         = self.current_odometry.twist.twist.angular
        act_ang_x, act_ang_y, act_ang_z = act_ang.x, act_ang.y, act_ang.z

        # ------------------------------------------------------------------
        # 2.  Build base row (everything *before* obstacles stays unchanged)
        # ------------------------------------------------------------------
        distance_to_goal = self.calculate_distance_to_goal()

        data_row = [
            current_time, elapsed_time,
            pos.x, pos.y, pos.z,
            goal_x, goal_y, goal_z,
            cmd_vel_x, cmd_vel_y, cmd_vel_z,
            des_vel_x, des_vel_y, des_vel_z,
            vel.x, vel.y, vel.z,
            distance_to_goal
        ]

        # ------------------------------------------------------------------
        # 3.  Obstacle positions (main + 0-9)  ────────
        # ------------------------------------------------------------------
        import numpy as np

        # Main obstacle
        if 'main' in self.obstacle_odometries:
            obs_pos = self.obstacle_odometries['main'].pose.pose.position
            data_row.extend([obs_pos.x, obs_pos.y, obs_pos.z])
        else:
            data_row.extend([np.nan, np.nan, np.nan])

        # Obstacles 0-9
        for i in range(10):
            if i in self.obstacle_odometries:
                obs_pos = self.obstacle_odometries[i].pose.pose.position
                data_row.extend([obs_pos.x, obs_pos.y, obs_pos.z])
            else:
                data_row.extend([np.nan, np.nan, np.nan])

        # ------------------------------------------------------------------
        # 4.  Append angular velocities (✱ NEW ✱)  ────
        # ------------------------------------------------------------------
        data_row.extend([
            cmd_ang_x, cmd_ang_y, cmd_ang_z,
            des_ang_x, des_ang_y, des_ang_z,
            act_ang_x, act_ang_y, act_ang_z
        ])

        # ------------------------------------------------------------------
        # 5.  Persist row
        # ------------------------------------------------------------------
        with open(self.csv_filename, 'a', newline='') as csvfile:
            csv.writer(csvfile).writerow(data_row)

        self.data_points.append(data_row)

        # ------------------------------------------------------------------
        # 6.  Periodic console status
        # ------------------------------------------------------------------
        if int(elapsed_time) % 5 == 0 and len(self.data_points) % 50 == 0:
            self.get_logger().info(
                f"[{elapsed_time:.1f}s] Pos: ({pos.x:.2f}, {pos.y:.2f}) | "
                f"Goal dist: {distance_to_goal:.2f} m | "
                f"Data points: {len(self.data_points)} | "
                f"Obstacles: {len(self.obstacle_odometries)}"
            )

    def _add_robot_fading_spheres(
        self, fig, time_stamps, pos_x, pos_y, pos_z,
        interval: float = 2.0, radius: float = 0.71, show_labels = True
    ):
        """
        Add semi-transparent blue spheres along the robot trajectory.
        Newer spheres are more opaque than older ones.
        """
        import plotly.graph_objects as go    # <---  make sure `go` is visible here
        import numpy as np

        # key-frames every `interval` seconds
        key_frames = []
        for i, t in enumerate(time_stamps):
            if not key_frames or t - key_frames[-1][0] >= interval:
                key_frames.append((t, pos_x[i], pos_y[i], pos_z[i]))

        t0, tN = key_frames[0][0], key_frames[-1][0]
        span    = max(tN - t0, 1e-6)         # avoid divide-by-zero

        u, v = np.mgrid[0:2*np.pi:20j, 0:np.pi:10j]   # sphere mesh
        for t, cx, cy, cz in key_frames:
            alpha = 0.1 + 0.4 * (t - t0) / span       # 0.1 → 0.5
            xs = cx + radius * np.cos(u) * np.sin(v)
            ys = cy + radius * np.sin(u) * np.sin(v)
            zs = cz + radius * np.cos(v)

            fig.add_trace(
                go.Surface(
                    x=xs, y=ys, z=zs,
                    opacity=alpha,
                    showscale=False,
                    colorscale=[[0, 'blue'], [1, 'blue']],
                    hoverinfo='skip',
                    name='Robot footprint'
                )
            )
            if show_labels:
                fig.add_trace(
                    go.Scatter3d(
                        x=[cx], y=[cy], z=[cz + radius + 0.2],
                        mode='text',
                        text=[f"{round(t - t0):d}s"],
                        textposition='top center',
                        textfont=dict(size=18, color='black'),
                        showlegend=False,
                        hoverinfo='skip'
                    )
                )

    def _add_obstacle_fading_spheres(
        self, fig, obstacle_data, time_stamps,
        interval: float = 2.0, radius: float = 0.50, show_labels = True
    ):
        """
        Add semi-transparent red spheres on every obstacle trajectory.
        Each obstacle gets its own fading series.
        `obstacle_data`  {label: (ox, oy)} – arrays must be same length as `time_stamps`.
        """
        import plotly.graph_objects as go    # <---  ensure available here too
        import numpy as np

        u, v = np.mgrid[0:2*np.pi:10j, 0:np.pi:5j]

        for label, (ox, oy, oz) in obstacle_data.items():
            u, v = np.mgrid[0:2*np.pi:10j, 0:np.pi:5j]
            key_frames = []

            for i, t in enumerate(time_stamps):
                if np.isnan(ox[i]) or np.isnan(oy[i]) or np.isnan(oz[i]):
                    continue
                if not key_frames or t - key_frames[-1][0] >= interval:
                    key_frames.append((t, ox[i], oy[i], oz[i]))

            if not key_frames:
                continue

            t0, tN = key_frames[0][0], key_frames[-1][0]
            span = max(tN - t0, 1e-6)

            for t, cx, cy, cz in key_frames:
                alpha = 0.1 + 0.4 * (t - t0) / span

                xs = cx + radius * np.cos(u) * np.sin(v)
                ys = cy + radius * np.sin(u) * np.sin(v)
                zs = cz + radius * np.cos(v)

                fig.add_trace(go.Surface(
                    x=xs, y=ys, z=zs,
                    opacity=alpha,
                    showscale=False,
                    colorscale=[[0, 'red'], [1, 'red']],
                    hoverinfo='skip',
                    name=f'Obstacle {label} footprint'
                ))

                if show_labels:
                    fig.add_trace(go.Scatter3d(
                        x=[cx], y=[cy], z=[cz + radius + 0.2],
                        mode='text',
                        text=[f"{round(t - t0):d}s"],
                        textposition='top center',
                        textfont=dict(size=28, color='darkred'),
                        showlegend=False,
                        hoverinfo='skip'
                    ))

    def shutdown_callback(self):
        """Called when node is shutting down"""
        self.get_logger().info(f"🛑 Shutting down. Logged {len(self.data_points)} data points")
        self.get_logger().info(f"📁 Data saved to: {self.csv_filename}")
        
        # Auto-generate plots if enabled and we have data
        if self.get_parameter('auto_generate_plots').value and len(self.data_points) > 0:
            self.get_logger().info("📊 Generating plots...")
            self.generate_plots(show_labels=True)
            self.generate_plots(show_labels=False)

            self.get_logger().info("🖼️ Generating 2-D XY plot ...")
            self.generate_xy_plot(show_labels=True)
            self.generate_xy_plot(show_labels=False)
            self.generate_xz_plot(show_labels=True)
            self.generate_xz_plot(show_labels=False)

            self.get_logger().info("📊 Generating Time PLots!")
            self.generate_velocity_magnitude_plot()
            self.generate_velocity_components_plot()
            #self.generate_cmd_vs_actual_plot()
            #self.generate_cmd_vs_actual_linear_angular_plot()

        if len(self.data_points) > 0:
            self.generate_stats_file()
   


    def generate_plots(self, show_labels=True):
        """
        Build an interactive Plotly 3-D scene:
        – blue robot trajectory + fading spheres (Ø 1.42 m)
        – red obstacle trajectories + fading spheres (Ø 1 m)
        – equal aspect ratio on all axes
        """
        try:
            import plotly.graph_objects as go
            import numpy as np
            import os

            data = np.asarray(self.data_points)
            if data.size == 0:
                self.get_logger().warn("No data to plot.")
                return

            # ───── Robot arrays ─────
            t    = data[:, 1]
            x_r  = data[:, 2]
            y_r  = data[:, 3]
            z_r  = data[:, 4]

            pad = 4.0
            
            x_min, x_max = np.min(x_r) - pad, np.max(x_r) + pad
            y_min, y_max = np.min(y_r) - pad, np.max(y_r) + pad
            z_min, z_max = np.min(z_r) - pad, np.max(z_r) + pad

            x_max = max(x_max, 5.0)
            x_min = min(x_min, -5.0)
            y_max = max(y_max, 5.0)
            y_min = min(y_min, -5.0)

            x_range = x_max - x_min
            y_range = y_max - y_min
            z_range = z_max - z_min
            max_range = max(x_range, y_range, z_range)

            aspect_ratio = dict(
                x=x_range / max_range,
                y=y_range / max_range,
                z=z_range / max_range
)

            robot_line = go.Scatter3d(
                x=x_r, y=y_r, z=z_r,
                mode='lines+markers',
                line=dict(color='blue', width=4),
                marker=dict(size=3, color='blue'),
                name='Robot path'
            )

            # ───── Obstacle arrays ─────
            obstacle_data_xyz = {}
            labels = ['main'] + [str(i) for i in range(10)]
            base = 18  # First obstacle x column
            for idx, label in enumerate(labels):
                x_idx = base + idx * 3
                y_idx = x_idx + 1
                z_idx = x_idx + 2
                if z_idx >= data.shape[1]:
                    break

                ox, oy, oz = data[:, x_idx], data[:, y_idx], data[:, z_idx]

                if np.isnan(ox).all() and np.isnan(oy).all() and np.isnan(oz).all():
                    continue

                obstacle_data_xyz[label] = (ox, oy, oz)

            self.get_logger().info(f"📦 Found {len(obstacle_data_xyz)} active obstacles (of {len(labels)})")

            obstacle_lines = []
            for lbl, value in obstacle_data_xyz.items():
                if not isinstance(value, tuple) or len(value) != 3:
                    self.get_logger().error(f"⚠️ Skipping malformed obstacle data for '{lbl}': {value}")
                    continue

                ox, oy, oz = value

                line = go.Scatter3d(
                    x=ox, y=oy, z=oz,
                    mode='lines',
                    line=dict(color='red', width=2, dash='dot'),
                    name=f'Obstacle {lbl}',
                    opacity=0.6
                )
                obstacle_lines.append(line)


            # ───── Assemble scene ─────
            fig = go.Figure(data=[robot_line] + obstacle_lines)

            # Fading spheres
            for label, value in obstacle_data_xyz.items():
                if not isinstance(value, tuple) or len(value) != 3:
                    self.get_logger().error(f"❌ Invalid obstacle data format for '{label}': {value}")
            self._add_robot_fading_spheres(fig, t, x_r, y_r, z_r,show_labels=show_labels)
            self._add_obstacle_fading_spheres(fig, obstacle_data_xyz, t, show_labels=show_labels)

            fig.update_layout(
                scene=dict(
                    xaxis=dict(
                        title='X [m]',
                        title_font=dict(size=20),
                        tickfont=dict(size=16),
                        tickmode='linear',
                        dtick=3.0,
                        tickangle=0,
                        range=[x_min, x_max]
                    ),
                    yaxis=dict(
                        title='Y [m]',
                        title_font=dict(size=20),
                        tickfont=dict(size=16),
                        tickmode='linear',
                        dtick=3.0,
                        tickangle=0,
                        range=[y_min, y_max]
                    ),
                    zaxis=dict(
                        title='Z [m]',
                        title_font=dict(size=20),
                        tickfont=dict(size=16),
                        tickmode='linear',
                        dtick=3.0,
                        tickangle=0,
                        range=[z_min, z_max]
                    ),
                    aspectmode='manual',
                    aspectratio=aspect_ratio
                ),
                legend=dict(font=dict(size=16)),
                margin=dict(l=0, r=0, b=0, t=0)
            )

            suffix = 'with_labels' if show_labels else 'no_labels'
            out_file = os.path.join(self.scenario_dir, f'trajectory_plot_3d_{suffix}.html')
            fig.write_html(out_file)
            self.get_logger().info(f"🌐 3-D plot written to {out_file}")

        except Exception as ex:
            self.get_logger().error(f"Plotly error: {ex}")

    def generate_xy_plot(self, show_labels=True):
        """Generate 2D XY trajectory plot with consistent visual scale and dynamic centering."""
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

            # --- Core arrays ---
            time_stamps = data[:, 1]
            pos_x, pos_y = data[:, 2], data[:, 3]
            goal_x, goal_y = data[:, 5], data[:, 6]



            # --- Crop data to visible XY range ---
            x_min, x_max = 0, 20
            y_min, y_max = -10, 10
            mask = (pos_x  >= x_min) & (pos_x <= x_max) & (pos_y >= y_min) & (pos_y <= y_max)
            data = data[mask, :]
            time_stamps = time_stamps[mask]
            pos_x, pos_y = pos_x[mask], pos_y[mask]
            goal_x, goal_y = goal_x[mask], goal_y[mask]

            for base_col in range(18, data.shape[1], 3):  # each obstacle has (x, y, z)
                ox = data[:, base_col]
                oy = data[:, base_col + 1]
                mask_obs = (ox < x_min) | (ox > x_max) | (oy < y_min) | (oy > y_max)
                data[mask_obs, base_col:base_col+2] = np.nan

            if time_stamps.size == 0:
                self.get_logger().warn("📊 XY plot skipped: no points inside visible range [-10,10]")
                return

            t0, tN = time_stamps[0], time_stamps[-1]
            span = max(tN - t0, 1e-6)
            alpha_min, alpha_max = 0.10, 0.75

            plt.figure(figsize=(10, 10))
            ax = plt.gca()

            agent_radius = 1.42 / 2
            obstacle_radius = 0.5
            circle_interval = 2.0
            last_time = -circle_interval

            obstacle_colors = ['orange', 'purple', 'brown', 'pink', 'gray',
                            'olive', 'cyan', 'magenta', 'yellow', 'lime', 'red']

            for i, t in enumerate(time_stamps):
                if t - last_time < circle_interval:
                    continue
                alpha = alpha_min + alpha_max * (t - t0) / span

                # --- Robot ---
                circle = Circle((pos_x[i], pos_y[i]), agent_radius,
                                edgecolor='blue', facecolor='blue',
                                alpha=alpha, linewidth=1)
                ax.add_patch(circle)
                if show_labels:
                    ax.text(pos_x[i], pos_y[i], f"{int(t)}s",
                            ha='center', va='center', fontsize=16, color='black')

                # --- Main obstacle ---
                if data.shape[1] > 20:
                    obs_x, obs_y = data[i, 18], data[i, 19]
                    if not np.isnan(obs_x) and not np.isnan(obs_y):
                        color = obstacle_colors[0]
                        circ = Circle((obs_x, obs_y), obstacle_radius,
                                    edgecolor=color, facecolor=color,
                                    alpha=alpha, linewidth=1)
                        ax.add_patch(circ)
                        if show_labels:
                            ax.text(obs_x, obs_y, f"{int(t)}s", ha='center', va='center',
                                    fontsize=16, color='black')

                # --- Obstacles 0–9 ---
                for j in range(10):
                    base = 21 + j * 3
                    if data.shape[1] <= base + 1:
                        break
                    ox, oy = data[i, base], data[i, base + 1]
                    if np.isnan(ox) or np.isnan(oy):
                        continue
                    color = obstacle_colors[(j + 1) % len(obstacle_colors)]
                    circ = Circle((ox, oy), obstacle_radius,
                                edgecolor=color, facecolor=color,
                                alpha=alpha, linewidth=1)
                    ax.add_patch(circ)
                    if show_labels:
                        ax.text(ox, oy, f"{int(t)}s",
                                ha='center', va='center', fontsize=16, color='black')

                last_time = t

            # --- Path and markers ---
            plt.plot(pos_x, pos_y, 'b--', linewidth=1, alpha=0.4)
            plt.scatter(pos_x[0], pos_y[0], color='blue', s=120, marker='s',
                        alpha=0.8, edgecolors='darkblue', linewidth=2, zorder=5)
            plt.scatter(goal_x[0], goal_y[0], color='red', s=150, marker='x',
                        linewidth=3, zorder=5)
            plt.scatter(pos_x[-1], pos_y[-1], color='blue', s=120, marker='s',
                        alpha=0.3, edgecolors='darkblue', linewidth=2, zorder=5)

            # --- Dynamic, consistent limits ---
            x_min, x_max = -10, 10
            y_min, y_max = -10, 10
            plt.xlim(x_min, x_max)
            plt.ylim(y_min, y_max)

            plt.grid(True, alpha=0.3)
            plt.xlabel('X Position (m)', fontsize=30, fontweight='bold')
            plt.ylabel('Y Position (m)', fontsize=30, fontweight='bold')
            plt.tick_params(axis='both', which='major', labelsize=30)
            plt.axis('equal')

            suffix = 'with_labels' if show_labels else 'no_labels'
            filename = os.path.join(self.scenario_dir, f'{self.scenario}_xy_trajectory_plot_{suffix}.png')
            plt.savefig(filename, dpi=300, bbox_inches='tight')
            plt.close()

            self.generate_stats_file()
            self.get_logger().info(f"📊 XY trajectory plot saved: {filename}")

        except Exception as e:
            self.get_logger().error(f"📊 Error generating XY plot: {e}")


    def generate_xz_plot(self, show_labels=True):
        """Generate 2D XZ footprint plot (same crop logic as XY plot)."""
        if not PLOTTING_AVAILABLE:
            self.get_logger().warn("📊 Could not generate plots: matplotlib not installed")
            return

        try:
            import matplotlib.pyplot as plt
            import numpy as np
            from matplotlib.patches import Circle

            data = np.array(self.data_points)
            if data.size == 0:
                self.get_logger().warn("📊 No data recorded – skipping XZ plot.")
                return

            # --- Core arrays ---
            time_stamps = data[:, 1]
            pos_x, pos_z = data[:, 2], data[:, 4]
            goal_x, goal_z = data[:, 5], data[:, 7]

            # --- Manual crop window (adjust for your scenario) ---
            x_min, x_max = 5, 15     # visible horizontal range
            z_min, z_max = -5, 5    # visible vertical range

            # --- Crop data to visible window ---
            mask = (pos_x >= x_min) & (pos_x <= x_max) & (pos_z >= z_min) & (pos_z <= z_max)
            data = data[mask, :]
            time_stamps = time_stamps[mask]
            pos_x, pos_z = pos_x[mask], pos_z[mask]
            goal_x, goal_z = goal_x[mask], goal_z[mask]

            # --- Crop obstacle data too ---
            for base_col in range(18, data.shape[1], 3):  # each obstacle has (x, y, z)
                ox = data[:, base_col]
                oz = data[:, base_col + 2]
                mask_obs = (ox < x_min) | (ox > x_max) | (oz < z_min) | (oz > z_max)
                data[mask_obs, base_col] = np.nan
                data[mask_obs, base_col + 2] = np.nan

            if time_stamps.size == 0:
                self.get_logger().warn(
                    f"📊 XZ plot skipped: no points inside visible range [{x_min},{x_max}]×[{z_min},{z_max}]"
                )
                return

            # --- Setup figure ---
            plt.figure(figsize=(10, 10))
            ax = plt.gca()

            agent_radius = 1.42 / 2
            obstacle_radius = 0.5
            circle_interval = 2.0
            last_time = -circle_interval

            t0, tN = time_stamps[0], time_stamps[-1]
            span = max(tN - t0, 1e-6)
            alpha_min, alpha_max = 0.10, 0.75

            obstacle_colors = [
                'orange', 'purple', 'brown', 'pink', 'gray',
                'olive', 'cyan', 'magenta', 'yellow', 'lime', 'red'
            ]

            # --- Draw circles every interval ---
            for i, t in enumerate(time_stamps):
                if t - last_time < circle_interval:
                    continue
                if not np.isfinite(pos_x[i]) or not np.isfinite(pos_z[i]):
                    continue

                alpha = alpha_min + alpha_max * (t - t0) / span

                # --- Robot ---
                circ = Circle((pos_x[i], pos_z[i]), agent_radius,
                              edgecolor='blue', facecolor='blue',
                              alpha=alpha, linewidth=1)
                ax.add_patch(circ)
                if show_labels:
                    ax.text(pos_x[i], pos_z[i], f"{int(t)}s",
                            ha='center', va='center', fontsize=16, color='black')

                # --- Main obstacle ---
                if data.shape[1] > 20:
                    obs_x, obs_z = data[i, 18], data[i, 20]
                    if np.isfinite(obs_x) and np.isfinite(obs_z):
                        color = obstacle_colors[0]
                        circ = Circle((obs_x, obs_z), obstacle_radius,
                                      edgecolor=color, facecolor=color,
                                      alpha=alpha, linewidth=1)
                        ax.add_patch(circ)
                        if show_labels:
                            ax.text(obs_x, obs_z, f"{int(t)}s",
                                    ha='center', va='center', fontsize=16, color='black')

                # --- Obstacles 0–9 ---
                for j in range(10):
                    base_col = 21 + j * 3
                    if data.shape[1] <= base_col + 2:
                        break
                    ox, oz = data[i, base_col], data[i, base_col + 2]
                    if not np.isfinite(ox) or not np.isfinite(oz):
                        continue
                    color = obstacle_colors[(j + 1) % len(obstacle_colors)]
                    circ = Circle((ox, oz), obstacle_radius,
                                  edgecolor=color, facecolor=color,
                                  alpha=alpha, linewidth=1)
                    ax.add_patch(circ)
                    if show_labels:
                        ax.text(ox, oz, f"{int(t)}s",
                                ha='center', va='center', fontsize=16, color='black')

                last_time = t

            # --- Path and markers ---
            plt.plot(pos_x, pos_z, 'b--', linewidth=1, alpha=0.4)
            plt.scatter(pos_x[0], pos_z[0], color='blue', s=120, marker='s',
                        alpha=0.8, edgecolors='darkblue', linewidth=2, zorder=5)
            plt.scatter(goal_x[0], goal_z[0], color='red', s=150, marker='x',
                        linewidth=3, zorder=5)
            plt.scatter(pos_x[-1], pos_z[-1], color='blue', s=120, marker='s',
                        alpha=0.3, edgecolors='darkblue', linewidth=2, zorder=5)

            # --- Axes / style ---
            plt.xlim(x_min, x_max)
            plt.ylim(z_min, z_max)
            plt.grid(True, alpha=0.3)
            plt.xlabel('X Position (m)', fontsize=30, fontweight='bold')
            plt.ylabel('Z Position (m)', fontsize=30, fontweight='bold')
            plt.tick_params(axis='both', which='major', labelsize=30)
            plt.axis('equal')

            suffix = 'with_labels' if show_labels else 'no_labels'
            filename = os.path.join(self.scenario_dir, f'{self.scenario}_xz_trajectory_plot_{suffix}.png')
            plt.savefig(filename, dpi=300, bbox_inches='tight')
            plt.close()

            self.get_logger().info(f"📊 XZ trajectory plot saved: {filename}")

        except Exception as e:
            self.get_logger().error(f"📊 XZ plot error: {e}")

    def generate_stats_file(self):
        """
        Generate performance stats.

        Average speed is now computed only over the cruise window defined by:
        – first time |v| > speed_threshold  (start_idx_cruise)
        – last  time |v| >= speed_threshold (end_idx_cruise)
        Any dips below the threshold between those two points are kept,
        so brief slow-downs do not split the window.
        """
        try:
            stats_filename = os.path.join(self.scenario_dir, 'performance_stats.txt')
            if not self.data_points:
                return

            data = np.array(self.data_points)
            t         = data[:, 1]        # elapsed time
            px, py, pz = data[:, 2], data[:, 3], data[:, 4]
            vx, vy, vz = data[:, 14], data[:, 15], data[:, 16]

            # ------------------------------------------------------------------
            # 1.  Identify mission-wide start / end indices
            # ------------------------------------------------------------------
            start_idx = 0
            end_idx   = len(data) - 1     # may shorten if goal reached

            # Goal pose (fallback if missing)
            if self.goal_pose_from_topic:
                gx, gy, gz = (self.goal_pose_from_topic.x,
                            self.goal_pose_from_topic.y,
                            self.goal_pose_from_topic.z)
            else:
                gx, gy, gz = 10.0, 0.0, 0.0

            # Clearance uses center-to-center distance minus agent/obstacle radii,
            # then adds back the protective zone to report surface clearance.
            robot_radius = 0.867
            for i in range(len(data)):
                if np.linalg.norm([px[i]-gx, py[i]-gy, pz[i]-gz]) <= robot_radius:
                    end_idx = i
                    break

            # ------------------------------------------------------------------
            # 2.  Mission-wide metrics (path length only)
            # ------------------------------------------------------------------
            mission_dt = t[end_idx] - t[start_idx]
            dx = np.diff(px[start_idx:end_idx+1])
            dy = np.diff(py[start_idx:end_idx+1])
            dz = np.diff(pz[start_idx:end_idx+1])
            path_len = np.sum(np.sqrt(dx**2 + dy**2 + dz**2))

            # ------------------------------------------------------------------
            # 3.  Clearance check (per obstacle)
            # ------------------------------------------------------------------
            obstacle_radius = 0.50
            protective_zone = 0.50
            min_clear = {"main": float("inf")}
            for j in range(10):
                min_clear[j] = float("inf")

            for i in range(start_idx, end_idx+1):
                pos = np.array([px[i], py[i], pz[i]])
                # main obstacle
                if data.shape[1] > 20:
                    obs = data[i, 18:21]
                    if not np.isnan(obs).any():
                        min_clear["main"] = min(
                            min_clear["main"],
                            np.linalg.norm(pos - obs) - robot_radius - obstacle_radius + protective_zone
                        )
                # obstacle 0-9
                for j in range(10):
                    base = 21 + j*3
                    if data.shape[1] > base+2:
                        obs = data[i, base:base+3]
                        if not np.isnan(obs).any():
                            min_clear[j] = min(
                                min_clear[j],
                                np.linalg.norm(pos - obs) - robot_radius - obstacle_radius + protective_zone
                            )

            # ------------------------------------------------------------------
            # 4.  Write report (minimal)
            # ------------------------------------------------------------------
            with open(stats_filename, "w") as f:
                f.write(f"AVOA Performance Statistics – Scenario {self.scenario}\n")
                f.write("="*60 + "\n\n")

                f.write("PATH\n")
                f.write(f"Path Length:  {path_len:.2f} m\n")
                f.write("\n")

                f.write("SAFETY (MIN CLEARANCE PER OBSTACLE)\n")
                main_clear = min_clear["main"]
                if main_clear != float("inf"):
                    f.write(f"main: {main_clear:.2f} m\n")
                    if main_clear < 0:
                        f.write("⚠️ Collision detected with main obstacle\n")
                else:
                    f.write("main: no data\n")
                for j in range(10):
                    clear = min_clear[j]
                    if clear != float("inf"):
                        f.write(f"obstacle_{j}: {clear:.2f} m\n")
                        if clear < 0:
                            f.write(f"⚠️ Collision detected with obstacle_{j}\n")
                    else:
                        f.write(f"obstacle_{j}: no data\n")

                f.write("\nGenerated: " + datetime.now().strftime("%Y-%m-%d %H:%M:%S") + "\n")

            self.get_logger().info(f"📋 Stats written → {stats_filename}")

        except Exception as e:
            self.get_logger().error(f"Stats generation error: {e}")
            import traceback; self.get_logger().error(traceback.format_exc())


    def generate_velocity_magnitude_plot(self):
        """Plot linear velocity magnitude over time (actual vs desired) with publication-ready styling"""
        if not PLOTTING_AVAILABLE:
            self.get_logger().warn("📊 Skipping velocity magnitude plot: matplotlib not installed")
            return

        try:
            import matplotlib.pyplot as plt
            import numpy as np
            import os

            data = np.array(self.data_points)
            if data.shape[0] == 0:
                self.get_logger().warn("📊 No data points available for velocity magnitude plot.")
                return

            t = data[:, 1]  # elapsed time

            # Actual linear velocity magnitude
            v_x = data[:, 14]
            v_y = data[:, 15]
            v_z = data[:, 16]
            actual_speed = np.sqrt(v_x**2 + v_y**2 + v_z**2)

            # Desired linear velocity magnitude
            v_dx = data[:, 11]
            v_dy = data[:, 12]
            v_dz = data[:, 13]
            desired_speed = np.sqrt(v_dx**2 + v_dy**2 + v_dz**2)

            # Plot
            plt.figure(figsize=(10, 10))  # Larger size for documents
            plt.plot(t, desired_speed, linestyle='--', color='green', linewidth=2.5)
            plt.plot(t, actual_speed, color='blue', linewidth=2.5)

            plt.xlabel('Time [s]', fontsize=30, fontweight='bold')
            plt.ylabel('Speed [m/s]', fontsize=30, fontweight='bold')
            plt.title('Linear Velocity Magnitude Over Time', fontsize=30, fontweight='bold')
            plt.grid(True, alpha=0.4)
            plt.legend(fontsize=30, loc='upper right')
            plt.xticks(fontsize=30)
            plt.yticks(fontsize=30)
            plt.tight_layout()

            filename = os.path.join(self.scenario_dir, f'{self.scenario}_velocity_magnitude_plot.png')
            plt.savefig(filename, dpi=300, bbox_inches='tight')
            plt.close()

            self.get_logger().info(f"📈 Velocity magnitude plot saved to: {filename}")

        except Exception as e:
            self.get_logger().error(f"📉 Error generating velocity magnitude plot: {e}")

    def generate_velocity_components_plot(self):
        """Generate velocity component plots with synchronized y-scale and padded range."""
        if not PLOTTING_AVAILABLE:
            self.get_logger().warn("📊 Skipping velocity plots: matplotlib not installed")
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
            vx_d, vy_d, vz_d = data[:, 11], data[:, 12], data[:, 13]
            vx_a, vy_a, vz_a = data[:, 14], data[:, 15], data[:, 16]

            all_vels = np.hstack([
                vx_d, vy_d, vz_d,
                vx_a, vy_a, vz_a
            ])
            v_min = np.min(all_vels)
            v_max = np.max(all_vels)
            margin = 0.1 * max(abs(v_min), abs(v_max), 1.0)  # At least 10% or 0.1

            y_low = min(v_min, -1.0) - margin
            y_high = max(v_max, 1.0) + margin

            def subplot_velocity(t, actuals, desireds, labels, title, filename):
                fig, axs = plt.subplots(len(labels), 1, figsize=(10, 10), sharex=True)
                if len(labels) == 1:
                    axs = [axs]

                colors = ['tab:red', 'tab:green', 'tab:blue']

                for i, label in enumerate(labels):
                    axs[i].plot(t, desireds[i], '--', color=colors[i], linewidth=3.5)
                    axs[i].plot(t, actuals[i], '-', color=colors[i], linewidth=3.5)
                    axs[i].set_ylabel(f'v{label} [m/s]', fontsize=30)
                    axs[i].tick_params(axis='both', labelsize=30)
                    axs[i].grid(True, alpha=0.3)
                    axs[i].legend(fontsize=30)
                    axs[i].set_ylim(y_low, y_high)

                axs[-1].set_xlabel('Time [s]', fontsize=30)
                fig.suptitle(title, fontsize=30, fontweight='bold')
                fig.tight_layout(rect=[0, 0, 1, 0.96])

                out_path = os.path.join(self.scenario_dir, filename)
                plt.savefig(out_path, dpi=300, bbox_inches='tight')
                plt.close()
                self.get_logger().info(f"📈 Velocity subplot saved: {filename}")

            # ── Plot vx, vy, vz ──
            subplot_velocity(
                t,
                actuals=[vx_a, vy_a, vz_a],
                desireds=[vx_d, vy_d, vz_d],
                labels=['x', 'y', 'z'],
                title='3D Linear Velocity Components Over Time',
                filename=f'{self.scenario}_velocity_subplots_3d.png'
            )

            # ── Plot vx, vy only ──
            subplot_velocity(
                t,
                actuals=[vx_a, vy_a],
                desireds=[vx_d, vy_d],
                labels=['x', 'y'],
                title='2D Linear Velocity Components Over Time',
                filename=f'{self.scenario}_velocity_subplots_2d.png'
            )

            # ── Plot vx vs wz (angular z) ──
            fig, axs = plt.subplots(2, 1, figsize=(10, 8), sharex=True)

            axs[0].plot(t, vx_a, color='tab:blue', linewidth=3.5)
            axs[0].set_ylabel('vx [m/s]', fontsize=30)
            axs[0].tick_params(axis='both', labelsize=30)
            axs[0].grid(True, alpha=0.3)

            axs[1].plot(t, data[:, 59], color='tab:orange', linewidth=3.5)  # wz actual only
            axs[1].set_ylabel('wz [rad/s]', fontsize=30)
            axs[1].tick_params(axis='both', labelsize=30)
            axs[1].grid(True, alpha=0.3)

            axs[1].set_xlabel('Time [s]', fontsize=30)
            fig.suptitle('Linear vs Angular Velocity (Differential Drive)', fontsize=30, fontweight='bold')
            fig.tight_layout(rect=[0, 0, 1, 0.96])

            filename = f'{self.scenario}_velocity_vx_wz_plot.png'
            plt.savefig(os.path.join(self.scenario_dir, filename), dpi=300, bbox_inches='tight')
            plt.close()

            self.get_logger().info(f"📈 Differential drive velocity plot saved: {filename}")
        except Exception as e:
            self.get_logger().error(f"📉 Error in velocity subplot generation: {e}")

    def generate_cmd_vs_actual_plot(self):
        """Generate cmd_vel vs actual_vel plots, same style as velocity_components_plot, no labels."""
        if not PLOTTING_AVAILABLE:
            self.get_logger().warn("📊 Skipping cmd_vel plot: matplotlib not installed")
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
            vx_c, vy_c, vz_c = data[:, 8], data[:, 9], data[:, 10]
            vx_a, vy_a, vz_a = data[:, 14], data[:, 15], data[:, 16]

            all_vels = np.hstack([
                vx_c, vy_c, vz_c,
                vx_a, vy_a, vz_a
            ])
            v_min, v_max = np.min(all_vels), np.max(all_vels)
            margin = 0.1 * max(abs(v_min), abs(v_max), 1.0)
            y_low = min(v_min, -1.0) - margin
            y_high = max(v_max, 1.0) + margin

            fig, axs = plt.subplots(3, 1, figsize=(10, 10), sharex=True)
            colors = ['tab:red', 'tab:green', 'tab:blue']
            labels = ['x', 'y', 'z']
            cmd_vels = [vx_c, vy_c, vz_c]
            act_vels = [vx_a, vy_a, vz_a]

            for i in range(3):
                axs[i].plot(t, cmd_vels[i], '--', color=colors[i], linewidth=3.5)
                axs[i].plot(t, act_vels[i], '-', color=colors[i], linewidth=3.5)
                axs[i].set_ylabel(f'v{labels[i]} [m/s]', fontsize=30)
                axs[i].tick_params(axis='both', labelsize=30)
                axs[i].grid(True, alpha=0.3)
                axs[i].set_ylim(y_low, y_high)

            axs[-1].set_xlabel('Time [s]', fontsize=30)
            fig.tight_layout()

            out_path = os.path.join(self.scenario_dir, f'{self.scenario}_cmd_vs_actual_vel.png')
            plt.savefig(out_path, dpi=300, bbox_inches='tight')
            plt.close()
            self.get_logger().info(f"📈 Cmd vs Actual velocity plot saved: {out_path}")

        except Exception as e:
            self.get_logger().error(f"📉 Error in cmd_vel vs actual_vel plot generation: {e}")

    def generate_cmd_vs_actual_linear_angular_plot(self):
        """Plot cmd_vel vs actual_vel for linear x and angular z, styled for publication, no labels."""
        if not PLOTTING_AVAILABLE:
            self.get_logger().warn("📊 Skipping cmd vs actual plot: matplotlib not installed")
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
            vx_c, vx_a = data[:, 8], data[:, 14]
            wz_c, wz_a = data[:, 53], data[:, 59]

            fig, axs = plt.subplots(2, 1, figsize=(10, 8), sharex=True)

            axs[0].plot(t, vx_c, '--', color='tab:green', linewidth=3.5)
            axs[0].plot(t, vx_a, '-', color='tab:green', linewidth=3.5)
            axs[0].set_ylabel('vx [m/s]', fontsize=30)
            axs[0].tick_params(axis='both', labelsize=30)
            axs[0].grid(True, alpha=0.3)

            axs[1].plot(t, wz_c, '--', color='tab:orange', linewidth=3.5)
            axs[1].plot(t, wz_a, '-', color='tab:orange', linewidth=3.5)
            axs[1].set_ylabel('wz [rad/s]', fontsize=30)
            axs[1].tick_params(axis='both', labelsize=30)
            axs[1].grid(True, alpha=0.3)

            axs[1].set_xlabel('Time [s]', fontsize=30)
            fig.tight_layout()

            filename = f'{self.scenario}_cmd_vs_actual_vx_wz.png'
            out_path = os.path.join(self.scenario_dir, filename)
            plt.savefig(out_path, dpi=300, bbox_inches='tight')
            plt.close()

            self.get_logger().info(f"📈 Cmd vs Actual vx/wz plot saved: {filename}")
 

        except Exception as e:
            self.get_logger().error(f"📉 Error generating cmd vs actual vx/wz plot: {e}")

def main(args=None):
    rclpy.init(args=args)
    
    import sys, pathlib, os
    print("[METRICS] interpreter:", sys.executable)
    print("[METRICS] sys.path[0]:", sys.path[0])
    print("[METRICS] matplotlib location:",
        next((p for p in sys.path if (pathlib.Path(p)/'matplotlib').exists()), 'NOT FOUND'))


    metrics_node = BasicAVOAMetrics()
    
    try:
        rclpy.spin(metrics_node)
    except KeyboardInterrupt:
        pass
    finally:
        try:
            metrics_node.shutdown_callback()
        finally:
            metrics_node.destroy_node()
            if rclpy.ok():  # <- ADD THIS CHECK
                rclpy.shutdown()


if __name__ == '__main__':
    main()
