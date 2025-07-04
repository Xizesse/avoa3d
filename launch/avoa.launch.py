#!/usr/bin/env python3

import os
import yaml
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
        default_value='s0',
        description='Scenario number: s=static, d=dynamic.'
    )
    
    # New TF frame arguments
    fixed_frame_arg = DeclareLaunchArgument(
        'fixed_frame',
        default_value='world',
        description='Fixed/world reference frame'
    )
    agent_frame_arg = DeclareLaunchArgument(
        'agent_frame',
        default_value='base_link',
        description='Agent reference frame'
    )
    # Goal topic argument
    goal_topic_arg = DeclareLaunchArgument(
        'goal_topic',
        default_value='/goal_pose',
        description='RViz2 goal pose topic'
    )

    # Create LaunchConfigurations to reference the user's CLI/arg values
    name = LaunchConfiguration('name')
    config_file = LaunchConfiguration('config_file')
    namespace = LaunchConfiguration('namespace')
    log_level = LaunchConfiguration('log_level')
    scenario = LaunchConfiguration('scenario')

    # Build path to your ros_gz_bridge config in ~/ros_ws/src/avoa3d/config
    home_path = os.environ.get('HOME', '/tmp')
    bridge_config = os.path.join(
        home_path, 'ros_ws', 'src', 'avoa3d', 'config', 'nest_bridge_config.yaml'
    )

    # Load your params.yaml for the avoa3dnode
    pkg_share = get_package_share_directory('avoa3d')
    avoa_params_path = os.path.join(pkg_share, 'config', 'nest_params.yaml')
    
    # Read the params file to get launch configuration
    with open(avoa_params_path, 'r') as f:
        params = yaml.safe_load(f)
    
    # Extract launch configurations (default to True for essential nodes if not specified)
    launch_configs = params.get('/**', {}).get('ros__parameters', {}).get('launch_nodes', {})
    
    # Default all to True if not specified (for backward compatibility)
    launch_bridge = launch_configs.get('bridge_node', True)
    launch_avoa3d = launch_configs.get('avoa3d_node', True)  # Core node
    launch_obstacle_publisher = launch_configs.get('obstacle_publisher', False)
    launch_rviz_marker = launch_configs.get('rviz_marker', False)
    launch_test_w_goal = launch_configs.get('test_w_goal', False)
    launch_tf_publisher = launch_configs.get('tf_publisher', False)
    launch_thruster_controller = launch_configs.get('thruster_controller', False)
    launch_metrics = launch_configs.get('metrics_node', True)  # Added metrics support
    
    # Static Transform Publishers
    static_tf_map_odom = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_map_odom',
        arguments=['0', '0', '0', '0', '0', '0', 'map', 'odom'],
        parameters=[{'use_sim_time': False}]
    )
    
    static_tf_base_agent = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_base_agent',
        arguments=['0', '0', '0', '0', '0', '0', 'base_link', 'agent'],
        parameters=[{'use_sim_time': False}]
    )
    
    # Define all nodes (we'll only add them to the launch description if enabled)
    
    # ros_gz_bridge node
    bridge_node = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name=name,
        namespace=namespace,
        output='screen',
        arguments=['--ros-args', '--log-level', log_level],
        parameters=[
            {'use_sim_time': False},
            {'config_file': bridge_config}
        ]
    )

    # obstacle_publisher
    obstacle_publisher_node = Node(
        package='avoa3d',
        executable='obstacle_publisher',
        name='obstacle_publisher',
        output='screen',
        parameters=[
            {'use_sim_time': False},
            avoa_params_path,
        ]
    )

    # rviz_marker
    rviz_marker_node = Node(
        package='avoa3d',
        executable='rviz_marker',
        name='rviz_marker',
        output='screen',
        parameters=[
            {'use_sim_time': False},
            avoa_params_path,
        ]
    )

    # test_w_goal
    test_w_goal_node = Node(
        package='avoa3d',
        executable='test_w_goal',
        name='test_w_goal',
        output='screen',
        parameters=[
            {'use_sim_time': False},
            avoa_params_path,
        ]
    )

    # avoa3d node
    avoa3d_node = Node(
        package='avoa3d',
        executable='avoa3dnode',
        name='avoa3dnode',
        output='screen',
        parameters=[
            {'use_sim_time': False},
            avoa_params_path,
        ]
    )

    # tf publisher
    tf_publisher = Node(
        package='avoa3d',
        executable='tf_publisher',
        name='tf_publisher',
        output='screen',
        emulate_tty=True,
        parameters=[
            avoa_params_path,
        ]
    )

    # thruster controller
    thruster_controller = Node(
        package='avoa3d',
        executable='thruster_controller',
        name='thruster_controller',
        output='screen',
        emulate_tty=True,
        parameters=[
            avoa_params_path
        ]
    )

    # metrics3d node - Added from holonomic.launch.py
    metrics_node = Node(
        package='avoa3d',
        executable='metrics3d.py',
        name='metrics_node',
        output='screen',
        parameters=[
            {'use_sim_time': False},
            {'scenario': scenario},  # Pass the scenario parameter
            {'results_directory': os.path.join(os.environ.get('HOME', '/tmp'), 'ros_ws', 'src', 'avoa3d', 'results')},
            {'topics.desired_vel': params.get('/**', {}).get('ros__parameters', {}).get('topics', {}).get('desired_vel', '/model/agente/desired_vel')},
            {'topics.cmd_vel': params.get('/**', {}).get('ros__parameters', {}).get('topics', {}).get('cmd_vel', '/model/agente/cmd_vel')},
            {'topics.odometry': params.get('/**', {}).get('ros__parameters', {}).get('topics', {}).get('odometry', '/model/agente/odometry')},
            {'topics.goal_odometry': params.get('/**', {}).get('ros__parameters', {}).get('topics', {}).get('goal_odometry', '/model/goal/odometry')},
            avoa_params_path
        ]
        # shutdown_timeout=30
    )

    # Create an empty list for nodes and add the argument declarations
    nodes = [
        name_arg,
        config_file_arg,
        namespace_arg,
        log_level_arg,
        scenario_arg,  # Added scenario argument
        fixed_frame_arg,
        agent_frame_arg,
        goal_topic_arg,
        
        # Add static transforms (always needed for proper TF tree)
        static_tf_map_odom,
        static_tf_base_agent,
    ]
    
    # Conditionally add nodes based on configuration
    if launch_bridge:
        nodes.append(bridge_node)
    
    if launch_avoa3d:
        nodes.append(avoa3d_node)
    
    if launch_obstacle_publisher:
        nodes.append(obstacle_publisher_node)
    
    if launch_rviz_marker:
        nodes.append(rviz_marker_node)
    
    if launch_test_w_goal:
        nodes.append(test_w_goal_node)
    
    if launch_tf_publisher:
        nodes.append(tf_publisher)
    
    if launch_thruster_controller:
        nodes.append(thruster_controller)
    
    if launch_metrics:
        nodes.append(metrics_node)
    
    return LaunchDescription(nodes)