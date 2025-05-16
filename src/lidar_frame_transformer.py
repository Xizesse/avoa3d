#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import PointCloud2

class PointCloudFrameTransformer(Node):
    def __init__(self):
        super().__init__('pointcloud_frame_transformer')
        self.subscription = self.create_subscription(
            PointCloud2,
            'points',  # Subscribe to the original point cloud topic
            self.listener_callback,
            10)
        self.publisher = self.create_publisher(
            PointCloud2, 
            'points_fixed',  # Publish to a new fixed frame topic
            10)
        self.count = 0
        self.get_logger().info('PointCloud frame transformer started!')
        
    def listener_callback(self, msg):
        # Create new message (PointCloud2 is a complex message, so we'll modify it directly)
        # This is more efficient than copying all fields individually
        fixed_msg = msg
        
        # Just change the frame ID
        original_frame = msg.header.frame_id
        fixed_msg.header.frame_id = 'lidar'  # Set to your desired frame
        
        # Publish the modified message
        self.publisher.publish(fixed_msg)
        
        # Log occasionally to avoid flooding
        if self.count % 100 == 0:
            self.get_logger().info(f'Changed frame from {original_frame} to {fixed_msg.header.frame_id}')
        self.count += 1

def main(args=None):
    rclpy.init(args=args)
    transformer = PointCloudFrameTransformer()
    rclpy.spin(transformer)
    transformer.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()