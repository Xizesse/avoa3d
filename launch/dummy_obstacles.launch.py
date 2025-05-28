#!/usr/bin/env python3

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, LogInfo
from launch.substitutions import LaunchConfiguration, TextSubstitution
from launch.conditions import IfCondition
from launch_ros.actions import Node
from launch.substitutions import PythonExpression

def generate_launch_description():
    # Declare launch arguments
    scenario_arg = DeclareLaunchArgument(
        'scenario',
        default_value='0',
        description='Scenario number (0,1,2,3...)'
    )
    
    fixed_frame_arg = DeclareLaunchArgument(
        'fixed_frame',
        default_value='world',
        description='Fixed frame for obstacle positions'
    )
    
    agent_frame_arg = DeclareLaunchArgument(
        'agent_frame', 
        default_value='base_link',
        description='Agent frame for coordinate transformation'
    )

    # Get launch configuration values
    scenario = LaunchConfiguration('scenario')
    fixed_frame = LaunchConfiguration('fixed_frame')
    agent_frame = LaunchConfiguration('agent_frame')

    # Log which scenario is being launched
    log_scenario = LogInfo(
        msg=['Launching dummy obstacles scenario: ', scenario]
    )

    # Scenario 0: Single static obstacle at x=5
    dummy_obstacle_0 = Node(
        package='obstacle_spawner',
        executable='dummy_obstacle_publisher_0',
        name='dummy_obstacle_publisher_0',
        parameters=[
            {'fixed_frame': fixed_frame},
            {'agent_frame': agent_frame},
            {'obstacle_x': 5.0},
            {'obstacle_y': 0.0}, 
            {'obstacle_z': 0.0},
            {'obstacle_id': 1}
        ],
        condition=IfCondition(PythonExpression([scenario, " == '0'"]))
    )

    # Scenario 1: Multiple static obstacles (example for future)
    dummy_obstacle_1 = Node(
        package='obstacle_spawner', 
        executable='dummy_obstacle_publisher_1',
        name='dummy_obstacle_publisher_1',
        parameters=[
            {'fixed_frame': fixed_frame},
            {'agent_frame': agent_frame}
        ],
        condition=IfCondition(PythonExpression([scenario, " == '1'"]))
    )

    # Scenario 2: Moving obstacles (example for future)
    dummy_obstacle_2 = Node(
        package='obstacle_spawner',
        executable='dummy_obstacle_publisher_2', 
        name='dummy_obstacle_publisher_2',
        parameters=[
            {'fixed_frame': fixed_frame},
            {'agent_frame': agent_frame}
        ],
        condition=IfCondition(PythonExpression([scenario, " == '2'"]))
    )

    # Scenario 3: Complex scenario (example for future)
    dummy_obstacle_3 = Node(
        package='obstacle_spawner',
        executable='dummy_obstacle_publisher_3',
        name='dummy_obstacle_publisher_3', 
        parameters=[
            {'fixed_frame': fixed_frame},
            {'agent_frame': agent_frame}
        ],
        condition=IfCondition(PythonExpression([scenario, " == '3'"]))
    )

    return LaunchDescription([
        scenario_arg,
        fixed_frame_arg,
        agent_frame_arg,
        log_scenario,
        dummy_obstacle_0,
        dummy_obstacle_1,
        dummy_obstacle_2,
        dummy_obstacle_3,
    ])