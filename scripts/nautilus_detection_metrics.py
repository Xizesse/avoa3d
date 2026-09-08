#!/usr/bin/env python3
"""
Clearance check: subscribes to Lily's and the nautilus's odometry, prints
the configured parameters once, then a continuously updating line with
the current hull-to-hull clearance and the all-time lowest clearance seen.

clearance = distance(lily, nautilus) - lily_radius - nautilus_radius

Defaults come from lily/docs/parameter_review.csv, cross-checked against
avoa3d/launch/s3vo.launch.py:
  - lily_radius      = 1.5  (S3VO's vehicle_radius; NOT the 0.6 hull_radius
                              from VRX's Surface/buoyancy plugin)
  - nautilus_radius   = 2.5 (mission2's gazebo_obstacle_publisher.py value -
                              simple1/2/3 have no ground-truth radius wired
                              at all per the spreadsheet, which estimates
                              the mesh itself at ~1.0m instead; override
                              this if you know the real figure)
  - protective_zone   = 0.0 (ElementCharacteristicsStamped.protective_zone -
                              never populated in the current pipeline)
  - danger_margin     = 0.1 (radius_threshold in s3vo.launch.py)

Run directly, no rebuild needed after edits (only the first CMake
registration needed a build):
    ros2 run avoa3d nautilus_detection_metrics.py
"""

import math

import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy, QoSDurabilityPolicy, QoSHistoryPolicy
from nav_msgs.msg import Odometry


class ClearanceMonitor(Node):
    def __init__(self):
        super().__init__('nautilus_clearance_monitor')

        self.declare_parameter('lily_odom_topic', '/usv/lily/mavros/local_position/odom')
        self.declare_parameter('nautilus_odom_topic', '/model/nautilus/odometry')
        self.declare_parameter('lily_radius', 1.5)          # S3VO vehicle_radius
        self.declare_parameter('nautilus_radius', 2.5)      # see docstring caveat above
        self.declare_parameter('protective_zone', 0.0)      # never populated in the pipeline
        self.declare_parameter('danger_margin', 0.1)        # s3vo.launch.py radius_threshold

        self.lily_radius = self.get_parameter('lily_radius').value
        self.nautilus_radius = self.get_parameter('nautilus_radius').value
        self.protective_zone = self.get_parameter('protective_zone').value
        self.danger_margin = self.get_parameter('danger_margin').value

        print(
            "Parameters:\n"
            f"  lily_radius      = {self.lily_radius} m\n"
            f"  nautilus_radius  = {self.nautilus_radius} m\n"
            f"  protective_zone  = {self.protective_zone} m\n"
            f"  danger_margin    = {self.danger_margin} m\n"
        )

        qos = QoSProfile(
            reliability=QoSReliabilityPolicy.BEST_EFFORT,
            durability=QoSDurabilityPolicy.VOLATILE,
            history=QoSHistoryPolicy.KEEP_LAST,
            depth=10
        )

        self.lily_pos = None
        self.nautilus_pos = None
        self.min_clearance = None

        self.create_subscription(
            Odometry, self.get_parameter('lily_odom_topic').value, self.lily_cb, qos)
        self.create_subscription(
            Odometry, self.get_parameter('nautilus_odom_topic').value, self.nautilus_cb, qos)

    def lily_cb(self, msg):
        self.lily_pos = msg.pose.pose.position
        self.update_clearance()

    def nautilus_cb(self, msg):
        self.nautilus_pos = msg.pose.pose.position
        self.update_clearance()

    def update_clearance(self):
        if self.lily_pos is None or self.nautilus_pos is None:
            return

        dx = self.nautilus_pos.x - self.lily_pos.x
        dy = self.nautilus_pos.y - self.lily_pos.y
        distance = math.hypot(dx, dy)

        clearance = distance - self.lily_radius - self.nautilus_radius
        if self.min_clearance is None or clearance < self.min_clearance:
            self.min_clearance = clearance

        print(f"\rdistance={distance:7.3f} m | clearance={clearance:7.3f} m   "
              f"min_clearance={self.min_clearance:7.3f} m   ",
              end='', flush=True)


def main(args=None):
    rclpy.init(args=args)
    node = ClearanceMonitor()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    if node.min_clearance is not None:
        print(f"\n--- all-time lowest clearance: {node.min_clearance:.3f} m ---")
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
