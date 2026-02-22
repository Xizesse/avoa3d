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
    print("DEBUG: ---------------------------------------------------")
    print("DEBUG: STARTING RVO.LAUNCH.PY GENERATION")
    print("DEBUG: ---------------------------------------------------")
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
    pkg_share_avoa = get_package_share_directory('avoa3d')
    # pkg_share_rvo = get_package_share_directory('rvo2_ros2') # If needed
    
    params_path = os.path.join(pkg_share_avoa, 'config', 'holonomic_params2.yaml')
    
    # Load parameters from yaml
    with open(params_path, 'r') as f:
        params = yaml.safe_load(f)
    
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
            os.path.join(pkg_share_avoa, 'launch', 'ground_truth_obstacles.launch.py')
        )
    )

    # Static Transform Publishers
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

    # Dynamic Transform Publisher (odom->base_link)
    dynamic_tf_publisher = Node(
        package='avoa3d',
        executable='tf_publisher',
        name='tf_publisher',
        output='screen',
        emulate_tty=True,
        parameters=[
            {'use_sim_time': True},
            {'fixed_frame': fixed_frame},
            {'agent_frame': agent_frame},
            {'agent_odometry_topic': '/model/agente/odometry'}, 
        ]
    )

    # Bridge
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
    
    # -------------------------------------------------------------
    # RVO / Avoa3d Hybrid Setup
    # -------------------------------------------------------------
    
    # 1. Test with Goal (from avoa3d)
    # This publishes /model/agente/desired_vel based on /goal_pose
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
            # Map parameters just in case they are used, but primarily uses defaults/args
            {'topics.desired_vel': '/model/agente/desired_vel'}
        ]
    )
    
    # 2. RVO2 Node (from rvo2_ros2)
    # Subscribes: /model/agente/odometry, /model/agente/desired_vel, /obstacles_array
    # Publishes: /model/agente/cmd_vel_unfiltered
    rvo2_node = Node(
        package='rvo2_ros2',
        executable='rvo2_node',
        name='rvo2_node',
        output='screen',
        parameters=[{'use_sim_time': True}]
    )

    # 3. Obstacle Publisher (from avoa3d) 
    # Publishes /element_tracking/elements (remapped to /obstacles_array for RVO)
    obstacle_publisher_node = Node(
        package='avoa3d',
        executable='obstacle_publisher',
        name='obstacle_publisher',
        output='screen',
        parameters=[
            params_path,
            {'use_sim_time': True},
            {'num_obstacles': LaunchConfiguration('num_obstacles')}
        ],
        remappings=[
            ('/element_tracking/elements', '/obstacles_array')
        ]
    )

    # 4. Velocity Filter (from rvo2_ros2)
    # Subscribes /model/agente/cmd_vel_unfiltered
    # Publishes /model/agente/cmd_vel
    # The bridge picks up /model/agente/cmd_vel to send to Gazebo
    velocity_filter_node = Node(
        package='rvo2_ros2',
        executable='velocity_filter_node',
        name='velocity_filter_node',
        output='screen',
        parameters=[{'use_sim_time': True}]
    )



    # Helper to publish a static goal for testing
    static_goal_pub = ExecuteProcess(
        cmd=[
            'ros2', 'topic', 'pub', '-r', '1',
            '/goal_pose', 'geometry_msgs/msg/PoseStamped',
            '{header: {frame_id: "map"}, pose: {position: {x: 10.0, y: 0.0, z: 0.0}, orientation: {w: 1.0}}}'
        ],
        output='screen'
    )

    nodes = [
        name_arg,
        config_file_arg,
        namespace_arg,
        log_level_arg,
        scenario_arg,
        scenarios_dir_arg,
        fixed_frame_arg,
        agent_frame_arg,
        goal_topic_arg,
        
        gazebo_process,
        bridge_node,
        static_tf_map_odom,
        static_tf_base_agent,
        dynamic_tf_publisher,
        #rviz_node,
        ground_truth_obstacles_launch,
        
        test_w_goal_node,       
        obstacle_publisher_node,
        rvo2_node,              
        velocity_filter_node,   


        static_goal_pub
    ]



    return LaunchDescription(nodes)
