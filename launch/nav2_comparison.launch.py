#!/usr/bin/env python3

import os
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    OpaqueFunction,
    ExecuteProcess,
    GroupAction
)
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch.conditions import IfCondition
from launch_ros.actions import Node, PushRosNamespace
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # Declare arguments
    scenario_arg = DeclareLaunchArgument(
        'scenario',
        default_value='s0',
        description='Scenario number: s=static, d=dynamic, followed by number.'
    )
    
    use_sim_time_arg = DeclareLaunchArgument(
        'use_sim_time',
        default_value='true',
        description='Use simulation time if true'
    )
    
    autostart_arg = DeclareLaunchArgument(
        'autostart', 
        default_value='true',
        description='Automatically start up the nav2 stack'
    )

    # Create LaunchConfigurations
    scenario = LaunchConfiguration('scenario')
    use_sim_time = LaunchConfiguration('use_sim_time')
    autostart = LaunchConfiguration('autostart')

    # Function to pick an SDF file based on scenario
    def get_sdf_file(context):
        scenario_value = context.launch_configurations['scenario']
        home = os.environ.get('HOME', '/tmp')
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

    # Use OpaqueFunction to dynamically pick the SDF file at runtime
    gazebo_process = OpaqueFunction(function=get_sdf_file)

    # Bridge configuration
    home_path = os.environ.get('HOME', '/tmp')
    bridge_config = os.path.join(
        home_path, 'ros_ws', 'src', 'avoa3d', 'config', 'bridge_config.yaml'
    )

    # ros_gz_bridge node
    bridge_node = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name='ros_gz_bridge',
        output='screen',
        arguments=['--ros-args', '--log-level', 'info'],
        parameters=[
            {'use_sim_time': use_sim_time},
            {'config_file': bridge_config}
        ]
    )

    # Obstacle publisher with topic remapping (assuming this publishes obstacles to the correct format for Nav2)
    obstacle_publisher_node = Node(
        package='avoa3d',
        executable='obstacle_publisher',
        name='obstacle_publisher',
        output='screen',
        parameters=[{'use_sim_time': use_sim_time}],
        # Add remapping if needed from your bridge topics to what Nav2 expects
        remappings=[
            # Map from obstacle publisher topics to nav2 expected topics
            # Example: ('/obstacle_topic', '/scan')
        ]
    )

    # Get the Nav2 params file
    pkg_share = get_package_share_directory('avoa3d')
    
    # We'll create a basic Nav2 params file (you'll need to customize this)
    nav2_params_path = os.path.join(pkg_share, 'config', 'nav2_params.yaml')
    
    # If the file doesn't exist, we'll need to create one (see below for contents)
    
    # Nav2 Controller
    controller_node = Node(
        package='nav2_controller',
        executable='controller_server',
        output='screen',
        parameters=[nav2_params_path],
        remappings=[
            ('cmd_vel', '/diff_drive/cmd_vel'), # Adjust this to your robot's command velocity topic
            ('odom', '/diff_drive/odometry')    # Adjust this to your robot's odometry topic
        ]
    )
    
    # Nav2 Planner
    planner_node = Node(
        package='nav2_planner',
        executable='planner_server',
        name='planner_server',
        output='screen',
        parameters=[nav2_params_path]
    )
    
    # Nav2 Behaviors
    bt_navigator_node = Node(
        package='nav2_bt_navigator',
        executable='bt_navigator',
        name='bt_navigator',
        output='screen',
        parameters=[nav2_params_path],
        remappings=[
            ('cmd_vel', '/diff_drive/cmd_vel')  # Adjust to match your robot's command velocity topic
        ]
    )
    
    # Nav2 Lifecycle Manager
    lifecycle_manager_node = Node(
        package='nav2_lifecycle_manager',
        executable='lifecycle_manager',
        name='lifecycle_manager_navigation',
        output='screen',
        parameters=[
            {'use_sim_time': use_sim_time},
            {'autostart': autostart},
            {'node_names': ['controller_server', 
                           'planner_server', 
                           'bt_navigator']}
        ]
    )
    
    # RViz configuration
    rviz_config_path = os.path.join(pkg_share, 'config', 'nav2_default_view.rviz')
    if not os.path.exists(rviz_config_path):
        rviz_config_path = ''  # Will use RViz's default configuration
    
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', rviz_config_path] if os.path.exists(rviz_config_path) else [],
        parameters=[{'use_sim_time': use_sim_time}]
    )

    return LaunchDescription([
        # Declare arguments
        scenario_arg,
        use_sim_time_arg,
        autostart_arg,
        
        # Simulation and bridge
        gazebo_process,
        bridge_node,
        obstacle_publisher_node,
        
        # Nav2 components
        controller_node,
        planner_node,
        bt_navigator_node,
        lifecycle_manager_node,
        
        # Visualization
        rviz_node
    ])