# avoa3d

ROS2 package implementing a 3D velocity obstacle avoidance algorithm. Given the robot's odometry, a desired velocity, and a list of tracked obstacles, it computes a collision-free velocity command.

## Dependencies

- ROS2 (Jazzy or later)
- `nav_msgs`, `geometry_msgs`, `sensor_msgs`, `visualization_msgs`, `tf2_ros`

## Build

```bash
cd ~/ros2_ws
colcon build --packages-select avoa3d
source install/setup.bash
```

## Launch

```bash
ros2 launch avoa3d s3vo.launch.py
```

All parameters can be overridden from the command line:

```bash
ros2 launch avoa3d s3vo.launch.py kinematic_mode:=holonomic v_x_max:=2.0
```

## Topics

| Direction | Topic | Type | Description |
|-----------|-------|------|-------------|
| Subscribe | `topics.odometry` | `nav_msgs/Odometry` | Robot odometry (actual velocity used for sampling) |
| Subscribe | `topics.desired_vel` | `geometry_msgs/TwistStamped` | Desired velocity goal |
| Subscribe | `topics.element_tracking` | `avoa3d/ElementCharacteristicsArray` | Tracked obstacles |
| Subscribe | `topics.goal_odometry` | `nav_msgs/Odometry` | Goal pose odometry |
| Publish | `topics.cmd_vel` | `geometry_msgs/TwistStamped` | Collision-free velocity command |

Default topic names are set in `avoa3dnode.cpp` and can be remapped via the parameter file or launch arguments.

## Key Parameters

### Kinematic mode

| Value | Description |
|-------|-------------|
| `holonomic` | Uniform random sampling in a 3D velocity box |
| `holonomic_ellipsoidal` | Sampling within an ellipsoidal velocity space |
| `diff_drive` | Differential drive (v, ω) space mapped to 2D Cartesian |

### Motion limits

| Parameter | Description |
|-----------|-------------|
| `v_x_max`, `v_y_max`, `v_z_max` | Maximum linear velocities (m/s) |
| `w_yaw_max` | Maximum yaw rate (rad/s, diff_drive only) |
| `a_x_max`, `a_y_max`, `a_z_max` | Maximum linear accelerations (m/s²) |
| `a_yaw_max` | Maximum yaw acceleration (rad/s², diff_drive only) |
| `delta_t` | Time horizon for dynamic window (s) |
| `num_samples` | Number of velocity samples per cycle |

### Safety

| Parameter | Description |
|-----------|-------------|
| `vehicle_radius` | Robot radius added to obstacle radii (m) |
| `time_to_collision_threshold` | Prune samples that collide within this time (s) |
| `radius_threshold` | Danger cone proximity threshold (m) |
| `watchdog_timeout` | Zero command if odometry or desired vel goes stale (s) |

### Cost weights

All weights are in `[0, 1]`. The final cost blends goal terms and a danger term:

| Parameter | Description |
|-----------|-------------|
| `heading_weight` | Alignment with desired velocity direction |
| `abs_weight` | Match desired velocity magnitude |
| `momentum_weight` | Penalise direction change from current velocity |
| `linear_matching_weight` | Euclidean distance to desired velocity vector |
| `twist_matching_weight` | Match desired linear and angular velocity (diff_drive) |
| `danger_weight` | Weight of obstacle danger vs. goal cost |

See `config/holonomic_params.yaml` or `config/diff_drive_params.yaml` for ready-to-use parameter sets.

## Obstacle Interface

Obstacles are provided via the custom `avoa3d/ElementCharacteristicsArray` message. Each element carries:

- `pose` — position in the agent frame
- `size` — bounding box (x, y, z)
- `velocity` — obstacle velocity in the agent frame
- `protective_zone` — extra safety margin added to the obstacle radius

## Visualisation

Two optional helper nodes are provided:

- **`rviz_marker`** — publishes agent and obstacle markers for RViz
- **`element_markers`** — publishes obstacle bounding-box markers

The avoa3dnode itself publishes the velocity sample cloud on `/avoa/velocity_samples` (PointCloud2) for direct inspection in RViz. Load `config/rviz_config.rviz` to get a pre-configured view.
