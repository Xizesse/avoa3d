#!/usr/bin/env python3

"""
Dummy Obstacle Publisher - Scenario 0
Publishes simple odometry for obstacle_1 at x=5, y=0, z=0 (world frame)
"""

import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Point, Quaternion, Vector3, Twist, Pose, PoseWithCovariance, TwistWithCovariance
from std_msgs.msg import Header

class DummyObstaclePublisher0(Node):
    def __init__(self):
        super().__init__('dummy_obstacle_publisher_0')
        
        # Declare parameters
        self.declare_parameter('obstacle_x', 5.0)
        self.declare_parameter('obstacle_y', 0.0)
        self.declare_parameter('obstacle_z', 0.0)
        self.declare_parameter('publish_rate', 10.0)  # Hz
        self.declare_parameter('frame_id', 'world')
        
        # Get parameters
        self.obstacle_x = self.get_parameter('obstacle_x').get_parameter_value().double_value
        self.obstacle_y = self.get_parameter('obstacle_y').get_parameter_value().double_value
        self.obstacle_z = self.get_parameter('obstacle_z').get_parameter_value().double_value
        self.frame_id = self.get_parameter('frame_id').get_parameter_value().string_value
        publish_rate = self.get_parameter('publish_rate').get_parameter_value().double_value
        
        self.get_logger().info(f'Dummy Obstacle Publisher Scenario 0 Starting')
        self.get_logger().info(f'Publishing obstacle_1 odometry at: ({self.obstacle_x}, {self.obstacle_y}, {self.obstacle_z})')
        self.get_logger().info(f'Frame: {self.frame_id}')
        
        # Create publisher for obstacle_1 odometry
        self.publisher_ = self.create_publisher(
            Odometry,
            '/model/obstacle_1/odometry',
            10
        )
        
        # Create timer for publishing
        timer_period = 1.0 / publish_rate  # seconds
        self.timer = self.create_timer(timer_period, self.publish_odometry)
        
        self.get_logger().info(f'Publishing at {publish_rate} Hz to /model/obstacle_1/odometry')

    def publish_odometry(self):
        """
        Publish simple odometry message for static obstacle
        """
        try:
            current_time = self.get_clock().now()
            
            # Create odometry message
            odom_msg = Odometry()
            
            # Header
            odom_msg.header.stamp = current_time.to_msg()
            odom_msg.header.frame_id = self.frame_id
            odom_msg.child_frame_id = 'obstacle_1'
            
            # Position (static obstacle at specified location)
            odom_msg.pose.pose.position.x = self.obstacle_x
            odom_msg.pose.pose.position.y = self.obstacle_y
            odom_msg.pose.pose.position.z = self.obstacle_z
            
            # Orientation (identity quaternion - no rotation)
            odom_msg.pose.pose.orientation.x = 0.0
            odom_msg.pose.pose.orientation.y = 0.0
            odom_msg.pose.pose.orientation.z = 0.0
            odom_msg.pose.pose.orientation.w = 1.0
            
            # Velocity (zero - static obstacle)
            odom_msg.twist.twist.linear.x = 0.0
            odom_msg.twist.twist.linear.y = 0.0
            odom_msg.twist.twist.linear.z = 0.0
            odom_msg.twist.twist.angular.x = 0.0
            odom_msg.twist.twist.angular.y = 0.0
            odom_msg.twist.twist.angular.z = 0.0
            
            # Optional: Set covariance (can leave as zeros for dummy data)
            # odom_msg.pose.covariance = [0.0] * 36
            # odom_msg.twist.covariance = [0.0] * 36
            
            # Publish
            self.publisher_.publish(odom_msg)
            
            self.get_logger().debug(
                f'Published obstacle_1 odometry at ({self.obstacle_x}, {self.obstacle_y}, {self.obstacle_z})'
            )
            
        except Exception as e:
            self.get_logger().error(f'Error publishing odometry: {e}')

def main(args=None):
    rclpy.init(args=args)
    
    dummy_publisher = DummyObstaclePublisher0()
    
    try:
        rclpy.spin(dummy_publisher)
    except KeyboardInterrupt:
        dummy_publisher.get_logger().info('Shutting down...')
    finally:
        dummy_publisher.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()