#!/usr/bin/env python3

import os
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    OpaqueFunction,
    ExecuteProcess
)
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # Declare arguments that match your XML launch file
    name_arg = DeclareLaunchArgument(
        'name',
        default_value='ros_gz_bridge'
    )
    config_file_arg = DeclareLaunchArgument(
        'config_file',
        default_value='config/bridge_config.yaml'
    )
    namespace_arg = DeclareLaunchArgument(
        'namespace',
        default_value=''
    )
    log_level_arg = DeclareLaunchArgument(
        'log_level',
        default_value='info'
    )
    scenario_arg = DeclareLaunchArgument(
        'scenario',
        default_value='1',
        description='Scenario number: s=static, d=dynamic.'
    )

    # Create LaunchConfigurations to reference the user’s CLI/arg values
    name = LaunchConfiguration('name')
    config_file = LaunchConfiguration('config_file')
    namespace = LaunchConfiguration('namespace')
    log_level = LaunchConfiguration('log_level')
    scenario = LaunchConfiguration('scenario')

    # Function to pick an SDF file based on scenario
    def get_sdf_file(context):
        scenario_value = context.launch_configurations['scenario']
        home = os.environ.get('HOME', '/tmp')  # or fallback
        sdf_base = os.path.join(home, 'ros_ws', 'src', 'avoa3d', 'sdf')

        scenario_map = {
            's0': 'single_static1.sdf',
            's1': 'single_static2.sdf',
            's2': 'single_static3.sdf',
            'd0': 'single_dynamic1.sdf',
            'd1': 'single_dynamic2.sdf',
            'd2': 'single_dynamic3.sdf',
            'd3': 'single_dynamic4.sdf',
            'd4': 'single_dynamic5.sdf',
        }
        chosen_sdf = scenario_map.get(scenario_value, 'single_static1.sdf')
        sdf_file = os.path.join(sdf_base, chosen_sdf)

        return [
            ExecuteProcess(
                cmd=['gz', 'sim', '4', '-r', sdf_file],
                output='screen'
            )
        ]

    # Use OpaqueFunction so we can dynamically pick the SDF file at runtime
    gazebo_process = OpaqueFunction(function=get_sdf_file)

    # Build path to your ros_gz_bridge config in ~/ros_ws/src/avoa3d/config
    home_path = os.environ.get('HOME', '/tmp')
    bridge_config = os.path.join(
        home_path, 'ros_ws', 'src', 'avoa3d', 'config', 'bridge_config.yaml'
    )

    # ros_gz_bridge node
    bridge_node = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name=name,
        namespace=namespace,
        output='screen',
        # Log level from user’s arg
        arguments=['--ros-args', '--log-level', log_level],
        parameters=[
            {'use_sim_time': True},
            {'config_file': bridge_config}
        ]
    )

    # obstacle_publisher
    obstacle_publisher_node = Node(
        package='avoa3d',
        executable='obstacle_publisher',
        name='obstacle_publisher',
        output='screen',
        parameters=[{'use_sim_time': True}]
    )

    # rviz_marker
    rviz_marker_node = Node(
        package='avoa3d',
        executable='rviz_marker',
        name='rviz_marker',
        output='screen',
        parameters=[{'use_sim_time': True}]
    )

    # test_w_goal
    test_w_goal_node = Node(
        package='avoa3d',
        executable='test_w_goal',
        name='test_w_goal',
        output='screen',
        parameters=[{'use_sim_time': True}]
    )

    # Load your holonomic_params.yaml for the avoa3dnode
    pkg_share = get_package_share_directory('avoa3d')
    motion_params = os.path.join(pkg_share, 'config', 'diff_drive_params.yaml')

    avoa3d_node = Node(
        package='avoa3d',
        executable='avoa3dnode',
        name='avoa3dnode',
        output='screen',
        parameters=[
            {'use_sim_time': True},
            motion_params
        ]
    )

    return LaunchDescription([
        # Declare arguments
        name_arg,
        config_file_arg,
        namespace_arg,
        log_level_arg,
        scenario_arg,

        # Processes & nodes
        gazebo_process,
        bridge_node,
        obstacle_publisher_node,
        rviz_marker_node,
        test_w_goal_node,
        avoa3d_node
    ])
