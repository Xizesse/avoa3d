#!/usr/bin/env python3

"""
Dummy Obstacle Publisher - Scenario 1
Publishes dynamic odometry for obstacle_1 starting at x=5, y=10, z=0
Moving at 1 m/s in positive Y direction
"""

import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Point, Quaternion, Vector3, Twist, Pose, PoseWithCovariance, TwistWithCovariance
from std_msgs.msg import Header
import time

class DummyObstaclePublisher1(Node):
    def __init__(self):
        super().__init__('dummy_obstacle_publisher_1')
        
        # Declare parameters
        self.declare_parameter('start_x', 5.0)
        self.declare_parameter('start_y', 10.0)
        self.declare_parameter('start_z', 0.0)
        self.declare_parameter('velocity_x', 0.0)
        self.declare_parameter('velocity_y', -1.0)  # 1 m/s in positive Y
        self.declare_parameter('velocity_z', 0.0)
        self.declare_parameter('publish_rate', 10.0)  # Hz
        self.declare_parameter('frame_id', 'world')
        
        # Get parameters
        self.start_x = self.get_parameter('start_x').get_parameter_value().double_value
        self.start_y = self.get_parameter('start_y').get_parameter_value().double_value
        self.start_z = self.get_parameter('start_z').get_parameter_value().double_value
        self.velocity_x = self.get_parameter('velocity_x').get_parameter_value().double_value
        self.velocity_y = self.get_parameter('velocity_y').get_parameter_value().double_value
        self.velocity_z = self.get_parameter('velocity_z').get_parameter_value().double_value
        self.frame_id = self.get_parameter('frame_id').get_parameter_value().string_value
        publish_rate = self.get_parameter('publish_rate').get_parameter_value().double_value
        
        # Initialize position tracking
        self.current_x = self.start_x
        self.current_y = self.start_y
        self.current_z = self.start_z
        
        # Time tracking for position integration
        self.start_time = time.time()
        self.last_time = self.start_time
        
        self.get_logger().info(f'Dummy Obstacle Publisher Scenario 1 (Dynamic) Starting')
        self.get_logger().info(f'Starting position: ({self.start_x}, {self.start_y}, {self.start_z})')
        self.get_logger().info(f'Velocity: ({self.velocity_x}, {self.velocity_y}, {self.velocity_z}) m/s')
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

    def update_position(self):
        """
        Update obstacle position based on velocity and time elapsed
        """
        current_time = time.time()
        dt = current_time - self.last_time  # Time increment
        
        # Update position using velocity * time
        self.current_x += self.velocity_x * dt
        self.current_y += self.velocity_y * dt
        self.current_z += self.velocity_z * dt
        
        # Update last time
        self.last_time = current_time
        
        # Log position every 5 seconds for debugging
        elapsed_total = current_time - self.start_time
        if int(elapsed_total) % 5 == 0 and int(elapsed_total) != getattr(self, '_last_log_time', -1):
            self._last_log_time = int(elapsed_total)
            self.get_logger().info(
                f'Position update: ({self.current_x:.2f}, {self.current_y:.2f}, {self.current_z:.2f}) '
                f'after {elapsed_total:.1f}s'
            )

    def publish_odometry(self):
        """
        Publish odometry message for moving obstacle
        """
        try:
            # Update position first
            self.update_position()
            
            current_time = self.get_clock().now()
            
            # Create odometry message
            odom_msg = Odometry()
            
            # Header
            odom_msg.header.stamp = current_time.to_msg()
            odom_msg.header.frame_id = self.frame_id
            odom_msg.child_frame_id = 'obstacle_1'
            
            # Position (updated based on velocity and time)
            odom_msg.pose.pose.position.x = self.current_x
            odom_msg.pose.pose.position.y = self.current_y
            odom_msg.pose.pose.position.z = self.current_z
            
            # Orientation (identity quaternion - no rotation)
            odom_msg.pose.pose.orientation.x = 0.0
            odom_msg.pose.pose.orientation.y = 0.0
            odom_msg.pose.pose.orientation.z = 0.0
            odom_msg.pose.pose.orientation.w = 1.0
            
            # Velocity (constant velocity)
            odom_msg.twist.twist.linear.x = self.velocity_x
            odom_msg.twist.twist.linear.y = self.velocity_y
            odom_msg.twist.twist.linear.z = self.velocity_z
            odom_msg.twist.twist.angular.x = 0.0
            odom_msg.twist.twist.angular.y = 0.0
            odom_msg.twist.twist.angular.z = 0.0
            
            # Optional: Set covariance (can leave as zeros for dummy data)
            # odom_msg.pose.covariance = [0.0] * 36
            # odom_msg.twist.covariance = [0.0] * 36
            
            # Publish
            self.publisher_.publish(odom_msg)
            
            self.get_logger().debug(
                f'Published obstacle_1 odometry at ({self.current_x:.2f}, {self.current_y:.2f}, {self.current_z:.2f})'
            )
            
        except Exception as e:
            self.get_logger().error(f'Error publishing odometry: {e}')

def main(args=None):
    rclpy.init(args=args)
    
    dummy_publisher = DummyObstaclePublisher1()
    
    try:
        rclpy.spin(dummy_publisher)
    except KeyboardInterrupt:
        dummy_publisher.get_logger().info('Shutting down...')
    finally:
        dummy_publisher.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()