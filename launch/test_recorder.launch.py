from launch import LaunchDescription
from launch_ros.actions import Node
import os

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='avoa3d',
            executable='recorder.py',
            name='data_recorder_test',
            output='screen',
            parameters=[
                {'use_sim_time': True},
                {'scenario': 'test'},
                {'algorithm': 'test_algo'},
                {'results_directory': os.path.join(os.environ.get('HOME', '/tmp'), 'ros2_ws', 'src', 'avoa3d', 'results')},
            ]
        )
    ])
