from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # 1. Separate bridge for Obstacle Ground Truth
        Node(
            package='ros_gz_bridge',
            executable='parameter_bridge',
            name='obstacle_bridge',
            arguments=[
                '/model/DURIUS/odometry@nav_msgs/msg/Odometry@gz.msgs.Odometry',
                '/model/nautilus/odometry@nav_msgs/msg/Odometry@gz.msgs.Odometry',
                '/model/buoy_south_30m/odometry@nav_msgs/msg/Odometry@gz.msgs.Odometry',
                '/model/buoy_west_20m/odometry@nav_msgs/msg/Odometry@gz.msgs.Odometry',
                '/model/nautilys_north_40m/odometry@nav_msgs/msg/Odometry@gz.msgs.Odometry'
            ],
            output='screen'
        ),

        # 2. Mock Tracker node that calculates relative positions
        Node(
            package='avoa3d',
            executable='gazebo_obstacle_publisher.py',
            name='gazebo_obstacle_publisher',
            parameters=[{
                'agent_odom_topic': '/usv/lily/mavros/local_position/odom',
                'durius_odom_topic': '/model/DURIUS/odometry',
                'nautilus_odom_topic': '/model/nautilus/odometry',
                'buoy_south_odom_topic': '/model/buoy_south_30m/odometry',
                'buoy_west_odom_topic': '/model/buoy_west_20m/odometry',
                'nautilys_north_odom_topic': '/model/nautilys_north_40m/odometry',
                'output_topic': '/element_tracking/elements',
                'durius_radius': 8.0,
                'nautilus_radius': 2.0,
                'obstacle_radius': 0.5
            }],
            output='screen'
        )
    ])
