#!/usr/bin/env python3

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction, ExecuteProcess
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # Declare arguments
    scenario_arg = DeclareLaunchArgument(
        'scenario',
        default_value='s1',
        description='Scenario to launch: s=static, d=dynamic, c=complex, v=viana'
    )
    log_level_arg = DeclareLaunchArgument(
        'log_level',
        default_value='info'
    )

    scenario = LaunchConfiguration('scenario')
    log_level = LaunchConfiguration('log_level')

    home_path = os.environ.get('HOME', '/tmp')
    sdf_base = os.path.join(home_path, 'ros2_ws', 'src', 'rvo2_ros2', 'sdf')

    # Scenario mapping
    scenario_map = {
        's1': 'single_static1.sdf',
        'd0': 'single_dynamic0.sdf',
        'd1': 'single_dynamic1.sdf',
        'd2': 'single_dynamic2.sdf',
        'd3': 'single_dynamic3.sdf',
        'c0': 'complex0.sdf',
        'c1': 'complex1.sdf',
        'c2': 'complex2.sdf',
        'c3': 'complex3.sdf',
        'c4': 'complex4.sdf',
        'c5': 'complex5.sdf',
        'c6': 'complex6.sdf',
        'c7': 'complex7.sdf',
        'c8': 'complex8.sdf',
        'c9': 'complex9.sdf',
        'v0': 'viana0.sdf',
    }

    # Function to launch Gazebo with selected scenario
    def launch_gazebo(context):
        scenario_value = context.launch_configurations['scenario']
        sdf_file = scenario_map.get(scenario_value, 'single_static1.sdf')
        sdf_path = os.path.join(sdf_base, sdf_file)

        return [
            ExecuteProcess(
                cmd=['gz', 'sim', '-r', sdf_path],
                output='screen'
            )
        ]

    gazebo_process = OpaqueFunction(function=launch_gazebo)

    # Bridge configuration paths
    bridge_config = os.path.join(
        home_path, 'ros2_ws', 'src', 'rvo2_ros2', 'config', 'bridge_config.yaml'
    )
    obstacle_bridge_config = os.path.join(
        home_path, 'ros2_ws', 'src', 'rvo2_ros2', 'config', 'obstacle_bridge.yaml'
    )

    # Main bridge node
    bridge_node = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name='ros_gz_bridge',
        output='screen',
        arguments=['--ros-args', '--log-level', log_level],
        parameters=[
            {'use_sim_time': True},
            {'config_file': bridge_config}
        ]
    )

    # Obstacle bridge node
    obstacle_bridge_node = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name='ros_gz_obstacle_bridge',
        output='screen',
        arguments=['--ros-args', '--log-level', log_level],
        parameters=[
            {'use_sim_time': True},
            {'config_file': obstacle_bridge_config}
        ]
    )

    return LaunchDescription([
        scenario_arg,
        log_level_arg,
        gazebo_process,
        bridge_node,
        obstacle_bridge_node
    ])
