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
        headers.extend(['obstacle_main_x', 'obstacle_main_y', 'obstacle_main_z'])
        for i in range(10):
            headers.extend([f'obstacle_{i}_x', f'obstacle_{i}_y', f'obstacle_{i}_z'])
        
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
        import numpy as np

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


    # ------------------------------------------------------------------
    # 2.  Obstacle fading spheres  (Ø 1 m → radius 0.5 m, red)
    # ------------------------------------------------------------------
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
                    if show_labels:
                        ax.text(pos_x[i], pos_y[i], f"{int(t)}s", ha='center', va='center',
                                fontsize=16, color='black')

                    # === Main obstacle circle ===
                    if data.shape[1] > 19:
                        obs_main_x = data[i, 18]
                        obs_main_y = data[i, 19]
                        obs_main_z = data[i, 20]
                        if obs_main_x != 0.0 or obs_main_y != 0.0:
                            circle = Circle((obs_main_x, obs_main_y), radius=obstacle_radius,
                                            edgecolor=obstacle_colors[0], facecolor=obstacle_colors[0],
                                            alpha=0.3, linewidth=1)
                            ax.add_patch(circle)
                            if show_labels:
                                ax.text(obs_main_x, obs_main_y, f"{int(t)}s", ha='center', va='center',
                                        fontsize=16, color='black')

                    # === Obstacles 0–9 ===
                    for j in range(10):
                        col_x = 21 + j * 3
                        col_y = col_x + 1
                        if data.shape[1] > col_y:
                            obs_x = data[i, col_x]
                            obs_y = data[i, col_y]
                            if obs_x != 0.0 or obs_y != 0.0:
                                color = obstacle_colors[(j + 1) % len(obstacle_colors)]
                                circle = Circle((obs_x, obs_y), radius=obstacle_radius,
                                                edgecolor=color, facecolor=color,
                                                alpha=0.3, linewidth=1)
                                ax.add_patch(circle)
                                if show_labels:
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

            suffix = 'with_labels' if show_labels else 'no_labels'
            filename = os.path.join(self.scenario_dir, f'xy_trajectory_plot_{suffix}.png')
            plt.savefig(filename, dpi=300, bbox_inches='tight')
            plt.close()

            self.generate_stats_file()
            self.get_logger().info(f"📊 Trajectory + labeled footprints saved to: {plot_filename}")

        except Exception as e:
            self.get_logger().error(f"📊 Error generating plots: {str(e)}")

    def generate_xz_plot(self, show_labels=True):
        """Generate an X-Z 2-D footprint plot (similar style to XY)."""
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

            # ── Core arrays ──────────────────────────────────────────────
            time_stamps = data[:, 1]
            pos_x = data[:, 2]
            pos_z = data[:, 4]
            goal_x = data[:, 5]
            goal_z = data[:, 7]

            plt.figure(figsize=(10, 10))
            ax = plt.gca()

            agent_radius     = 1.42 / 2
            obstacle_radius  = 0.5
            circle_interval  = 2.0   # seconds between circles
            last_time        = -circle_interval

            obstacle_colors = [
                'orange', 'purple', 'brown', 'pink', 'gray',
                'olive', 'cyan', 'magenta', 'yellow', 'lime', 'red'
            ]

            # ── Draw circles every `circle_interval` seconds ─────────────
            for i, t in enumerate(time_stamps):
                if t - last_time < circle_interval:
                    continue

                # Robot footprint
                circ = Circle((pos_x[i], pos_z[i]), agent_radius,
                            edgecolor='blue', facecolor='blue',
                            alpha=0.30, linewidth=1)
                ax.add_patch(circ)
                if show_labels:
                    ax.text(pos_x[i], pos_z[i], f"{int(t)}s",
                            ha='center', va='center', fontsize=16, color='black')

                # Main obstacle (columns 18–19 = x, y)
                # Main obstacle (columns 18–20 = x, y, z)
                if data.shape[1] > 20:
                    obs_x = data[i, 18]
                    obs_z = data[i, 20]
                    if obs_x or obs_z:
                        circ = Circle((obs_x, obs_z), obstacle_radius,
                                    edgecolor=obstacle_colors[0],
                                    facecolor=obstacle_colors[0],
                                    alpha=0.30, linewidth=1)
                        ax.add_patch(circ)
                        if show_labels:
                            ax.text(obs_x, obs_z, f"{int(t)}s",
                                    ha='center', va='center', fontsize=16, color='black')

                        
                # Obstacles 0-9
                for j in range(10):
                    base_col = 21 + j * 3
                    if data.shape[1] <= base_col + 2:
                        break  # Skip if data incomplete
                    ox = data[i, base_col]
                    oz = data[i, base_col + 2]
                    if ox or oz:
                        color = obstacle_colors[(j + 1) % len(obstacle_colors)]
                        circ = Circle((ox, oz), obstacle_radius,
                                    edgecolor=color, facecolor=color,
                                    alpha=0.30, linewidth=1)
                        ax.add_patch(circ)
                        if show_labels:
                            ax.text(ox, oz, f"{int(t)}s",
                                    ha='center', va='center', fontsize=16, color='black')


                last_time = t

            # Dashed robot path
            plt.plot(pos_x, pos_z, 'b--', linewidth=1, alpha=0.4)

            # Start / goal / end markers
            plt.scatter(pos_x[0],  pos_z[0],  s=120, marker='s', color='blue',
                        alpha=0.8,  edgecolors='darkblue', linewidth=2, zorder=5)
            plt.scatter(goal_x[0], goal_z[0], s=150, marker='x', color='red',
                        linewidth=3, zorder=5)
            plt.scatter(pos_x[-1], pos_z[-1], s=120, marker='s', color='blue',
                        alpha=0.3,  edgecolors='darkblue', linewidth=2, zorder=5)

            # Axes / grid / style
            plt.xlim(-10, 10)
            plt.ylim(-10, 10)
            plt.grid(True, alpha=0.3)
            plt.xlabel('X Position (m)', fontsize=16, fontweight='bold')
            plt.ylabel('Z Position (m)', fontsize=16, fontweight='bold')
            plt.tick_params(axis='both', which='major', labelsize=14)
            plt.axis('equal')

            suffix = 'with_labels' if show_labels else 'no_labels'
            filename = os.path.join(self.scenario_dir, f'xz_trajectory_plot_{suffix}.png')
            plt.savefig(filename, dpi=300, bbox_inches='tight')
            plt.close()

            self.get_logger().info(f"📊 XZ footprint plot saved to: {out_png}")
            # stats file is already generated in other calls, no need to duplicate

        except Exception as e:
            self.get_logger().error(f"📊 XZ plot error: {e}")

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
            pos_z = data[:, 4]
            distance_to_goal = data[:, -1]  # Last column before obstacle data
            
            # Calculate statistics
            total_time = time_elapsed[-1]
            final_distance = distance_to_goal[-1] if not np.isinf(distance_to_goal[-1]) else float('inf')
            path_length = np.sum(np.sqrt(np.diff(pos_x)**2 + np.diff(pos_y)**2 + np.diff(pos_z)**2))

            min_distance_to_goal = np.min(distance_to_goal[~np.isinf(distance_to_goal)]) if np.any(~np.isinf(distance_to_goal)) else float('inf')
            average_speed = path_length / total_time if total_time > 0 else 0
            
            # Start and end positions
            start_pos = f"({pos_x[0]:.2f}, {pos_y[0]:.2f}, {pos_z[0]:.2f})"
            end_pos = f"({pos_x[-1]:.2f}, {pos_y[-1]:.2f}, {pos_z[-1]:.2f})"
            
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