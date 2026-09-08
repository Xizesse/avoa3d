#!/usr/bin/env python3
"""
Ground-truth obstacle publisher for the single 'nautilus' obstacle model
spawned in the simple1/2/3 regression worlds.

Tracks one Gazebo model (default name 'nautilus') via its ros_gz-bridged
/model/<name>/odometry topic and republishes it as an ElementCharacteristics
element on /element_tracking/elements, in the agent body frame: relative
position, and the obstacle's absolute velocity expressed in the agent frame
(the convention the sample evaluator expects). Nothing is published until
live odometry for the obstacle has been received — no stale/hardcoded
fallback pose.
"""

import math
import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from avoa3d.msg import ElementCharacteristicsArray, ElementCharacteristicsStamped


def get_yaw(q):
    return math.atan2(2.0 * (q.w * q.z + q.x * q.y),
                      1.0 - 2.0 * (q.y * q.y + q.z * q.z))


def world_to_body(dx, dy, yaw):
    c, s = math.cos(yaw), math.sin(yaw)
    return dx * c + dy * s, -dx * s + dy * c


def body_to_world(x, y, yaw):
    c, s = math.cos(yaw), math.sin(yaw)
    return x * c - y * s, x * s + y * c


class ObstaclePublisher(Node):

    def __init__(self):
        super().__init__('gazebo_obstacle_publisher')

        self.declare_parameter('obstacle_model_name', 'nautilus')
        self.declare_parameter('obstacle_radius', 2.5)
        self.declare_parameter('agent_odom_topic', '/usv/lily/mavros/local_position/odom')

        self._name   = self.get_parameter('obstacle_model_name').get_parameter_value().string_value
        self._radius = self.get_parameter('obstacle_radius').get_parameter_value().double_value
        agent_odom_topic = self.get_parameter('agent_odom_topic').get_parameter_value().string_value

        self._agent_x   = None
        self._agent_y   = None
        self._agent_yaw = None

        self._obs_x   = 0.0
        self._obs_y   = 0.0
        self._obs_vx  = 0.0
        self._obs_vy  = 0.0
        self._obs_live = False

        # Agent odometry
        self.create_subscription(Odometry, agent_odom_topic, self._agent_cb, 10)

        # Obstacle odometry
        obstacle_topic = f'/model/{self._name}/odometry'
        self.create_subscription(Odometry, obstacle_topic, self._odom_cb, 10)
        self.get_logger().info(f'Tracking {self._name} via {obstacle_topic} (r={self._radius}m)')

        self._pub = self.create_publisher(
            ElementCharacteristicsArray,
            '/element_tracking/elements', 10
        )

        self.create_timer(0.1, self._publish)
        self.get_logger().info(f'Ready — waiting for {agent_odom_topic} and {obstacle_topic} ...')

    def _agent_cb(self, msg):
        if self._agent_x is None:
            self.get_logger().info('Agent odometry received — starting to publish elements.')
        self._agent_x   = msg.pose.pose.position.x
        self._agent_y   = msg.pose.pose.position.y
        self._agent_yaw = get_yaw(msg.pose.pose.orientation)

    def _odom_cb(self, msg):
        self._obs_x = msg.pose.pose.position.x
        self._obs_y = msg.pose.pose.position.y
        # Odometry twist is expressed in the model body frame → rotate by the
        # model yaw to get world-ENU velocity
        yaw = get_yaw(msg.pose.pose.orientation)
        self._obs_vx, self._obs_vy = body_to_world(msg.twist.twist.linear.x,
                                                    msg.twist.twist.linear.y, yaw)
        if not self._obs_live:
            self._obs_live = True
            self.get_logger().info(
                f'{self._name}: odometry live — pos ({self._obs_x:.2f}, {self._obs_y:.2f}), '
                f'world vel ({self._obs_vx:.2f}, {self._obs_vy:.2f}) m/s'
            )

    def _publish(self):
        if self._agent_x is None or not self._obs_live:
            return

        dx, dy   = self._obs_x - self._agent_x, self._obs_y - self._agent_y
        rx, ry   = world_to_body(dx, dy, self._agent_yaw)
        rvx, rvy = world_to_body(self._obs_vx, self._obs_vy, self._agent_yaw)
        dynamic  = math.hypot(self._obs_vx, self._obs_vy) > 0.05

        el = ElementCharacteristicsStamped()
        el.header.stamp    = self.get_clock().now().to_msg()
        el.header.frame_id = 'base_link'
        el.id              = 0
        el.dynamic         = dynamic
        el.pose.position.x = rx
        el.pose.position.y = ry
        el.velocity.x      = rvx
        el.velocity.y      = rvy
        el.size.x = el.size.y = el.size.z = self._radius * 2.0

        out = ElementCharacteristicsArray()
        out.elements.append(el)
        self._pub.publish(out)


def main(args=None):
    rclpy.init(args=args)
    node = ObstaclePublisher()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
