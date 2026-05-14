import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    pkg_share = get_package_share_directory('avoa3d')
    
    # Default config file path
    default_config = os.path.join(pkg_share, 'config', 'static_obstacles.yaml')

    return LaunchDescription([
        # 1. Dummy Obstacle Publisher
        # Reads GPS coordinates from YAML and converts to ENU elements
        Node(
            package='avoa3d',
            executable='dummy_obstacle_publisher',
            name='dummy_obstacle_publisher',
            output='screen',
            parameters=[{
                'config_file': default_config,
                'origin_topic': '/usv/lily/mavros/global_position/gp_origin',
                'odometry_topic': '/usv/lily/mavros/local_position/odom',
                'agent_frame': 'base_link',
                'publish_rate': 10.0
            }]
        ),

        # 2. Element Markers
        # Converts Elements to RViz Markers for visualization
        Node(
            package='avoa3d',
            executable='element_markers',
            name='element_markers',
            output='screen',
            parameters=[{
                'elements_topic': '/element_tracking/elements',
                'markers_topic': '/markers/element_array',
                'frame_id': 'base_link'
            }]
        )
    ])
