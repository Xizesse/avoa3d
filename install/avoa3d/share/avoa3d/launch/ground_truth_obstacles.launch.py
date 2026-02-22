#!/usr/bin/env python3

import os
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    
    # Get package share directory and config paths
    pkg_share = get_package_share_directory('avoa3d')
    params_path = os.path.join(pkg_share, 'config', 'holonomic_params.yaml')
    
    # Bridge config path
    home_path = os.environ.get('HOME', '/tmp')
    bridge_config = os.path.join(
        home_path, 'ros2_ws', 'src', 'avoa3d', 'config', 'bridge_obstacles_config.yaml'
    )

    # Bridge node - connects Gazebo to ROS2
    bridge_node = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name='ros_gz_bridge',
        output='screen',
        parameters=[
            {'use_sim_time': True},
            {'config_file': bridge_config}
        ]
    )

    # Obstacle publisher node - provides ground truth obstacle information
    obstacle_publisher_node = Node(
        package='avoa3d',
        executable='obstacle_publisher',
        name='obstacle_publisher',
        output='screen',
        parameters=[
            {'use_sim_time': True},
            {'fixed_frame': 'map'},
            {'agent_frame': 'agent'},
            params_path
        ]
    )

    return LaunchDescription([
        bridge_node,
        obstacle_publisher_node,
    ])