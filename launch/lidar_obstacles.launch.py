#!/usr/bin/env python3

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    
    # 1. Lidar frame transformer (Python node)
    lidar_frame_transformer = Node(
        package='avoa3d',
        executable='laser_frame_transformer.py',
        output='screen'
    )

    # 2. Obstacle detector launch (from obstacle_detector package)
    obstacle_detector_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([
                FindPackageShare('obstacle_detector'),
                'launch',
                'my_obstacle_extractor_and_tracker.launch'
            ])
        )
    )

    # 3. Obstacle converter (C++ node)
    obstacle_converter = Node(
        package='avoa3d',
        executable='obstacle_converter_node',
        output='screen'
    )

    return LaunchDescription([
        lidar_frame_transformer,
        obstacle_detector_launch,
        obstacle_converter,
    ])