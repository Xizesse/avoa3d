#!/usr/bin/env python3

import os
import yaml
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    OpaqueFunction,
    ExecuteProcess,
    IncludeLaunchDescription
)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # Declare arguments
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
    
    # Frame arguments
    fixed_frame_arg = DeclareLaunchArgument(
        'fixed_frame',
        default_value='map',
        description='Fixed/world reference frame'
    )
    agent_frame_arg = DeclareLaunchArgument(
        'agent_frame',
        default_value='agent',
        description='Agent reference frame'
    )
    
    # Goal topic argument
    goal_topic_arg = DeclareLaunchArgument(
        'goal_topic',
        default_value='/goal_pose',
        description='RViz2 goal pose topic'
    )

    # Create LaunchConfigurations
    name = LaunchConfiguration('name')
    config_file = LaunchConfiguration('config_file')
    namespace = LaunchConfiguration('namespace')
    log_level = LaunchConfiguration('log_level')
    scenario = LaunchConfiguration('scenario')
    fixed_frame = LaunchConfiguration('fixed_frame')
    agent_frame = LaunchConfiguration('agent_frame')
    goal_topic = LaunchConfiguration('goal_topic')

    # Get package share directory and config paths
    pkg_share = get_package_share_directory('avoa3d')
    params_path = os.path.join(pkg_share, 'config', 'holonomic_params2.yaml')
    
    # Load parameters from yaml
    with open(params_path, 'r') as f:
        params = yaml.safe_load(f)
    
    # Extract launch configurations
    launch_configs = params.get('/**', {}).get('ros__parameters', {}).get('launch_nodes', {})
    
    # Check which nodes to launch
    launch_bridge = launch_configs.get('bridge_node', True)
    launch_avoa3d = launch_configs.get('avoa3d_node', True)
    launch_obstacle_publisher = launch_configs.get('obstacle_publisher', True)
    launch_rviz_marker = launch_configs.get('rviz_marker', True)
    launch_test_w_goal = launch_configs.get('test_w_goal', True)
    launch_thruster_controller = launch_configs.get('thruster_controller', False)
    launch_metrics = launch_configs.get('metrics_node', False)
    
    # NEW: Single flag to control all transforms
    publish_transforms = launch_configs.get('publish_transforms', True)
    
    # Bridge config path
    home_path = os.environ.get('HOME', '/tmp')
    bridge_config = os.path.join(
        home_path, 'ros2_ws', 'src', 'avoa3d', 'config', 'bridge_config.yaml'
    )

    rviz_config_path = os.path.join(
        home_path, 'ros2_ws', 'src', 'avoa3d', 'config', 'rviz_config.rviz')

    # Function to select the SDF file based on scenario parameter
    def get_gazebo_process(context):
        scenario_value = context.launch_configurations['scenario']
        sdf_base = os.path.join(home_path, 'ros2_ws', 'src', 'avoa3d', 'sdf')
        
        # Define scenario mapping here in the launch file
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
        
        chosen_sdf = scenario_map.get(scenario_value, 'single_static1.sdf')
        sdf_file = os.path.join(sdf_base, chosen_sdf)
        
        return [
            ExecuteProcess(
                cmd=['gz', 'sim', '4', '-r', sdf_file],
                output='screen'
            )
        ]

    # Use OpaqueFunction for dynamic gazebo process
    gazebo_process = OpaqueFunction(function=get_gazebo_process)

    ground_truth_obstacles_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_share, 'launch', 'ground_truth_obstacles.launch.py')
        )
    )

    # Static Transform Publishers (Standard ROS2 approach)
    static_tf_map_odom = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_map_odom',
        arguments=['0', '0', '0', '0', '0', '0', 'map', 'odom'],
        parameters=[{'use_sim_time': True}]
    )
    
    
    static_tf_base_agent = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_base_agent',
        arguments=['0', '0', '0', '0', '0', '0', 'base_link', 'agent'],
        parameters=[{'use_sim_time': True}]
    )

    # Dynamic Transform Publisher (for odom->base_link)
    dynamic_tf_publisher = Node(
        package='avoa3d',
        executable='tf_publisher',  # Keep your existing executable name
        name='tf_publisher',
        output='screen',
        emulate_tty=True,
        parameters=[
            {'use_sim_time': True},
            {'fixed_frame': fixed_frame},
            {'agent_frame': agent_frame},
            {'agent_odometry_topic': '/model/agente/odometry'},  # Make configurable
            params_path
        ]
    )

    # Regular nodes
    bridge_node = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name=name,
        namespace=namespace,
        output='screen',
        arguments=['--ros-args', '--log-level', log_level],
        parameters=[
            {'use_sim_time': True},
            {'config_file': bridge_config}
        ]
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config_path],
        output='screen',
        parameters=[{'use_sim_time': True}]
    )

    obstacle_publisher_node = Node(
        package='rvo2_ros2',
        executable='obstacle_publisher',
        name='obstacle_publisher',
        output='screen',
        parameters=[{'use_sim_time': True}]
    )

    rviz_marker_node = Node(
        package='avoa3d',
        executable='rviz_marker',
        name='rviz_marker',
        output='screen',
        parameters=[
            {'use_sim_time': True},
            {'fixed_frame': fixed_frame},
            {'agent_frame': agent_frame},
            params_path
        ]
    )

    test_w_goal_node = Node(
        package='avoa3d',
        executable='test_w_goal',
        name='test_w_goal',
        output='screen',
        parameters=[
            {'use_sim_time': True},
            {'fixed_frame': fixed_frame},
            {'agent_frame': agent_frame},
            {'goal_topic': goal_topic},
            params_path
        ]
    )

    rvo2_node = Node(
        package='rvo2_ros2',
        executable='rvo2_node',
        name='rvo2_node',
        output='screen',
        parameters=[{'use_sim_time': True}]
    )

    velocity_filter_node = Node(
        package='rvo2_ros2',
        executable='velocity_filter_node',
        name='velocity_filter_node',
        output='screen',
        parameters=[{'use_sim_time': True}]
    )
    
    thruster_controller_node = Node(
        package='avoa3d',
        executable='thruster_controller',
        name='thruster_controller',
        output='screen',
        emulate_tty=True,
        parameters=[
            {'use_sim_time': True},
            {'fixed_frame': fixed_frame},
            {'agent_frame': agent_frame},
            params_path
        ]
    )

    metrics_node = Node(
        package='avoa3d',
        executable='metrics3d_raw.py',
        name='metrics_raw_node',
        output='screen',
        parameters=[
            {'use_sim_time': True},
            {'scenario': scenario},
            {'results_directory': os.path.join(os.environ.get('HOME', '/tmp'), 'ros2_ws', 'src', 'avoa3d', 'results')},
            {'agent_odometry_topic': params.get('/**', {}).get('ros__parameters', {}).get('topics', {}).get('odometry', '/model/agente/odometry')},
            {'main_obstacle_topic': params.get('/**', {}).get('ros__parameters', {}).get('topics', {}).get('obstacle_odometry', '/model/obstacle/odometry')}
        ]
    )

    # Create an empty list for nodes and add the argument declarations
    nodes = [
        # Declare arguments
        name_arg,
        config_file_arg,
        namespace_arg,
        log_level_arg,
        scenario_arg,
        fixed_frame_arg,
        agent_frame_arg,
        goal_topic_arg,
        
        # Add gazebo process
        gazebo_process,
        ground_truth_obstacles_launch
    ]
    
    # Always add transform publishers when enabled (recommended for most use cases)
    if publish_transforms:
        nodes.extend([
            static_tf_map_odom,
            static_tf_base_agent,
            dynamic_tf_publisher
        ])
    
    # Conditionally add other nodes based on configuration
    if launch_bridge:
        nodes.append(bridge_node)
    
    if launch_avoa3d:
        nodes.append(rvo2_node)
        #nodes.append(velocity_filter_node)
    
    if launch_obstacle_publisher:
        nodes.append(obstacle_publisher_node)
    
    if launch_rviz_marker:
        nodes.append(rviz_marker_node)
        nodes.append(rviz_node)
    
    if launch_test_w_goal:
        nodes.append(test_w_goal_node)
        
    if launch_thruster_controller:
        nodes.append(thruster_controller_node)
    
    if launch_metrics:
        nodes.append(metrics_node)
    
    
    return LaunchDescription(nodes)
