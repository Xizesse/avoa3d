from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

# Bridge for mission2 obstacle odometry (harmless if topics don't exist)
_BRIDGE = [
    '/model/DURIUS/odometry@nav_msgs/msg/Odometry@gz.msgs.Odometry',
    '/model/nautilus_1/odometry@nav_msgs/msg/Odometry@gz.msgs.Odometry',
    '/model/nautilus_2/odometry@nav_msgs/msg/Odometry@gz.msgs.Odometry',
    '/model/nautilus_3/odometry@nav_msgs/msg/Odometry@gz.msgs.Odometry',
]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument('scenario', default_value='mission1',
                              description='mission1 or mission2'),

        Node(
            package='ros_gz_bridge',
            executable='parameter_bridge',
            name='obstacle_bridge',
            arguments=_BRIDGE,
            output='screen'
        ),

        Node(
            package='avoa3d',
            executable='gazebo_obstacle_publisher.py',
            name='gazebo_obstacle_publisher',
            parameters=[{'scenario': LaunchConfiguration('scenario')}],
            output='screen'
        ),
    ])
