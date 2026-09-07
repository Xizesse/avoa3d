import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # Define parameters with default values based on nest_params.yaml and avoa3dnode.cpp
    # Using dots in keys for nested parameters in the Node action
    params = {
        'fixed_frame': 'world',
        'agent_frame': 'base_link',
        'topics.desired_vel': '/desired_vel',
        'topics.cmd_vel': '/usv/lily/cmd_vel',
        'topics.odometry': '/usv/lily/mavros/local_position/odom',
        'topics.element_tracking': '/element_tracking/elements',
        'topics.goal_odometry': '/model/goal/odometry', #TODO : REMOVE THIS
        
        'kinematic_mode': 'diff_drive',
        'vehicle_radius': 1.5,

        'heading_weight': 0.0,
        'abs_weight': 0.0,
        'danger_weight': 0.2,
        'momentum_weight': 0.4,
        'linear_matching_weight': 0.0,
        'twist_matching_weight': 0.8,

        'a_x_max': 1.0,
        'a_y_max': 0.0,
        'a_z_max': 0.0,
        'a_roll_max': 0.0,
        'a_pitch_max': 0.0,
        'a_yaw_max': 0.5,
        'v_x_max': 1.5,
        'v_y_max': 0.0,
        'v_z_max': 0.0,
        'w_roll_max': 0.0,
        'w_pitch_max': 0.0,
        'w_yaw_max': 1.0,
        'delta_t': 1.0,
        'num_samples': 10000,
        'time_to_collision_threshold': 60.0,
        'radius_threshold': 0.1,
        'bypass_avoidance': True,
    }

    # Create LaunchArguments for each parameter
    # Arg names use underscores instead of dots to be CLI-friendly
    launch_args = []
    node_params = {}
    for key, default in params.items():
        arg_name = key.replace('.', '_')
        launch_args.append(DeclareLaunchArgument(
            arg_name,
            default_value=str(default),
            description=f'Parameter {key}'
        ))
        node_params[key] = LaunchConfiguration(arg_name)

    # Define the avoa3dnode
    avoa3d_node = Node(
        package='avoa3d',
        executable='avoa3dnode',
        name='avoa3dnode',
        output='screen',
        parameters=[node_params]
    )

    return LaunchDescription(launch_args + [avoa3d_node])
