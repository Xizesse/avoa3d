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
from launch.conditions import IfCondition
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
        default_value='0',
        description='Scenario number: integer index for generated scenarios.'
    )
    
    scenarios_dir_arg = DeclareLaunchArgument(
        'scenarios_dir',
        default_value='scenarios',
        description='Name of the scenarios directory'
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
    
    num_obstacles_arg = DeclareLaunchArgument(
        'num_obstacles',
        default_value='50',
        description='Number of obstacles to track'
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

    launch_velocity_filter = launch_configs.get('velocity_filter_node', True)
    
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
        scenarios_dir_name = context.launch_configurations['scenarios_dir']
        base_path = os.path.join(home_path, 'ros2_ws', 'src', 'avoa3d')
        scenarios_dir = os.path.join(base_path, scenarios_dir_name)
        
        try:
            scenario_idx = int(scenario_value)
            sdf_file_name = f"scenario_{scenario_idx:03d}.sdf"
            sdf_path = os.path.join(scenarios_dir, sdf_file_name)
            
            if not os.path.exists(sdf_path):
                 print(f"WARNING: Scenario file not found: {sdf_path}")

        except ValueError:
            print(f"ERROR: Scenario '{scenario_value}' is not a valid integer. Using default scenario_000.sdf")
            sdf_path = os.path.join(scenarios_dir, "scenario_000.sdf")
        
        gazebo_default_gui_config = os.path.join(
            home_path, 'ros2_ws', 'src', 'avoa3d', 'config', 'gz_window_layout.config'
        )

        return [
            ExecuteProcess(
                cmd=['gz', 'sim', '4', '-r', f'--gui-config={gazebo_default_gui_config}', sdf_path],
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
        package='avoa3d',
        executable='obstacle_publisher',
        name='obstacle_publisher',
        output='screen',
        parameters=[
            {'use_sim_time': True},
            {'fixed_frame': fixed_frame},
            {'agent_frame': agent_frame},
            {'num_obstacles': LaunchConfiguration('num_obstacles')},
            params_path
        ]
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

    avoa3d_node = Node(
        package='avoa3d',
        executable='avoa3dnode',
        name='avoa3dnode',
        output='screen',
        parameters=[
            {'use_sim_time': True},
            {'fixed_frame': fixed_frame},
            {'agent_frame': agent_frame},
            params_path
        ]
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



    # Create an empty list for nodes and add the argument declarations
    nodes = [
        # Declare arguments
        name_arg,
        config_file_arg,
        namespace_arg,
        log_level_arg,
        scenario_arg,
        scenarios_dir_arg,
        fixed_frame_arg,
        agent_frame_arg,
        goal_topic_arg,
        num_obstacles_arg,
        
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
        nodes.append(avoa3d_node)

    if launch_velocity_filter:
        nodes.append(velocity_filter_node)
    
    if launch_obstacle_publisher:
        nodes.append(obstacle_publisher_node)
    
    if launch_rviz_marker:
        nodes.append(rviz_marker_node)
        #nodes.append(rviz_node)
    
    if launch_test_w_goal:
        nodes.append(test_w_goal_node)
        
        # Publish static goal pose
        nodes.append(
            ExecuteProcess(
                cmd=[
                    'ros2', 'topic', 'pub', '-r', '1',
                    '/goal_pose', 'geometry_msgs/msg/PoseStamped',
                    '{header: {frame_id: "map"}, pose: {position: {x: 10.0, y: 0.0, z: 0.0}, orientation: {w: 1.0}}}'
                ],
                output='screen'
            )
        )
        
    if launch_thruster_controller:
        nodes.append(thruster_controller_node)
    

    
    return LaunchDescription(nodes)
