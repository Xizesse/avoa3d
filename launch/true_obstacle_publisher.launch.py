from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    obstacle_model_name = LaunchConfiguration('obstacle_model_name')
    obstacle_radius = LaunchConfiguration('obstacle_radius')

    return LaunchDescription([
        DeclareLaunchArgument('obstacle_model_name', default_value='nautilus',
                              description='Gazebo model name of the obstacle to track'),
        DeclareLaunchArgument('obstacle_radius', default_value='2.5',
                              description='Obstacle radius in meters'),

        Node(
            package='ros_gz_bridge',
            executable='parameter_bridge',
            name='obstacle_bridge',
            arguments=['/model/nautilus/odometry@nav_msgs/msg/Odometry@gz.msgs.Odometry'],
            output='screen'
        ),

        Node(
            package='avoa3d',
            executable='gazebo_obstacle_publisher.py',
            name='gazebo_obstacle_publisher',
            parameters=[{
                'obstacle_model_name': obstacle_model_name,
                'obstacle_radius': obstacle_radius,
            }],
            output='screen'
        ),
    ])
