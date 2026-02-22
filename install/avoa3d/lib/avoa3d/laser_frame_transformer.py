#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import LaserScan

class LaserFrameTransformer(Node):
    def __init__(self):
        super().__init__('laser_frame_transformer')
        self.subscription = self.create_subscription(
            LaserScan,
            'scan',
            self.listener_callback,
            10)
        self.publisher = self.create_publisher(LaserScan, 'scan_fixed', 10)
        self.get_logger().info('Laser frame transformer started!')
        
    def listener_callback(self, msg):
        # Copy the message and change the frame ID
        fixed_msg = LaserScan()
        fixed_msg.header.stamp = msg.header.stamp
        fixed_msg.header.frame_id = 'lidar'  # Change to your desired frame
        
        # Copy all other fields
        fixed_msg.angle_min = msg.angle_min
        fixed_msg.angle_max = msg.angle_max
        fixed_msg.angle_increment = msg.angle_increment
        fixed_msg.time_increment = msg.time_increment
        fixed_msg.scan_time = msg.scan_time
        fixed_msg.range_min = msg.range_min
        fixed_msg.range_max = msg.range_max
        fixed_msg.ranges = msg.ranges
        fixed_msg.intensities = msg.intensities
        
        self.publisher.publish(fixed_msg)
        if self.count % 100 == 0:  # Log occasionally to avoid flooding
            self.get_logger().info(f'Changed frame from {msg.header.frame_id} to {fixed_msg.header.frame_id}')
        self.count += 1

def main(args=None):
    rclpy.init(args=args)
    transformer = LaserFrameTransformer()
    transformer.count = 0
    rclpy.spin(transformer)
    transformer.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()