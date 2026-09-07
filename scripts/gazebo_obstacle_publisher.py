#!/usr/bin/env python3
"""
Per-scenario ground-truth obstacle publisher.

Set scenario = 'mission1' or 'mission2' via parameter.

mission1: all obstacles are Gazebo static models with no odometry, so their
world-ENU positions are hardcoded.

mission2: the subscription list is hardcoded (DURIUS + the 3 nautilus
vessels), but positions AND velocities are updated dynamically from the
ros_gz-bridged /model/<name>/odometry topics:
  * static models (DURIUS, nautilus_1) fall back to their SDF spawn pose
    until/unless their odometry arrives;
  * moving vessels (nautilus_2, nautilus_3) are only published once live
    odometry is received — their pose/velocity is never hardcoded.

Elements are published in the agent body frame: relative position, and the
obstacle's absolute velocity expressed in the agent frame (the convention
the sample evaluator expects).
"""

import math
import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from avoa3d.msg import ElementCharacteristicsArray, ElementCharacteristicsStamped

# ---------------------------------------------------------------------------
# Gazebo world-ENU positions for static obstacles
# (origin: lat=41.685699, lon=-8.838574, same as SDF spherical_coordinates)
# ---------------------------------------------------------------------------
STATIC_MISSION1 = [
    {'name': 'DURIUS',          'x': -2.57,  'y':  0.44,  'r': 8.0},
    {'name': 'buoy_left',       'x': -12.57, 'y':  0.44,  'r': 0.5},
    {'name': 'buoy_top',        'x': -2.57,  'y': 13.44,  'r': 0.5},  # midpoint green-yellow
    {'name': 'buoy_right',      'x': 13.43,  'y':  0.44,  'r': 0.5},
    {'name': 'buoy_bottom',     'x': -2.57,  'y': -9.56,  'r': 0.5},
    {'name': 'buoy_bottom_far', 'x': -2.57,  'y': -15.56, 'r': 0.5},  # 6m further from DURIUS
]

# ---------------------------------------------------------------------------
# mission2: /model/<name>/odometry is bridged for every entry below.
# x0/y0 = SDF spawn pose, used only as fallback for models whose odometry
# never arrives (Gazebo static models). Entries with moving=True are not
# published until their odometry is live, so a moving vessel is never
# reported at a stale hardcoded position.
# ---------------------------------------------------------------------------
TRACKED_MISSION2 = [
    {'name': 'DURIUS',     'r': 8.0, 'x0': -2.57,  'y0':   0.44, 'moving': False},
    {'name': 'nautilus_1', 'r': 2.5, 'x0': -2.57,  'y0':  13.44, 'moving': False},
    {'name': 'nautilus_2', 'r': 2.5, 'x0': 10.43,  'y0':  11.69, 'moving': True},
    {'name': 'nautilus_3', 'r': 2.5, 'x0': -22.57, 'y0': -12.56, 'moving': True},
]


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

        self.declare_parameter('scenario', 'mission1')
        scenario = self.get_parameter('scenario').get_parameter_value().string_value

        self.get_logger().info(f'Starting obstacle publisher — scenario: {scenario}')

        if scenario == 'mission2':
            self._static  = []
            self._tracked = {t['name']: {'x': t['x0'], 'y': t['y0'],
                                         'vx': 0.0, 'vy': 0.0,
                                         'r': t['r'], 'moving': t['moving'],
                                         'live': False}
                             for t in TRACKED_MISSION2}
        else:
            self._static  = STATIC_MISSION1
            self._tracked = {}

        self._agent_x   = None
        self._agent_y   = None
        self._agent_yaw = None

        # Agent odometry
        self.create_subscription(
            Odometry,
            '/usv/lily/mavros/local_position/odom',
            self._agent_cb, 10
        )

        # Obstacle odometry — position and velocity both come from the bridge
        for name, obs in self._tracked.items():
            topic = f'/model/{name}/odometry'
            self.create_subscription(
                Odometry, topic,
                lambda msg, n=name: self._odom_cb(msg, n),
                10
            )
            self.get_logger().info(
                f'Tracking {name} via {topic} '
                f'(fallback pose ({obs["x"]:.2f}, {obs["y"]:.2f})'
                f'{", withheld until odometry" if obs["moving"] else ""})'
            )

        self._pub = self.create_publisher(
            ElementCharacteristicsArray,
            '/element_tracking/elements', 10
        )

        for obs in self._static:
            self.get_logger().info(
                f'Static obstacle: {obs["name"]} at ({obs["x"]}, {obs["y"]}) r={obs["r"]}'
            )

        self.create_timer(0.1, self._publish)
        self.get_logger().info('Ready — waiting for /usv/lily/mavros/local_position/odom ...')

    def _agent_cb(self, msg):
        if self._agent_x is None:
            self.get_logger().info('Agent odometry received — starting to publish elements.')
        self._agent_x   = msg.pose.pose.position.x
        self._agent_y   = msg.pose.pose.position.y
        self._agent_yaw = get_yaw(msg.pose.pose.orientation)

    def _odom_cb(self, msg, name):
        obs = self._tracked[name]
        obs['x'] = msg.pose.pose.position.x
        obs['y'] = msg.pose.pose.position.y
        # Odometry twist is expressed in the model body frame → rotate by the
        # model yaw to get world-ENU velocity
        yaw = get_yaw(msg.pose.pose.orientation)
        obs['vx'], obs['vy'] = body_to_world(msg.twist.twist.linear.x,
                                             msg.twist.twist.linear.y, yaw)
        if not obs['live']:
            obs['live'] = True
            self.get_logger().info(
                f'{name}: odometry live — pos ({obs["x"]:.2f}, {obs["y"]:.2f}), '
                f'world vel ({obs["vx"]:.2f}, {obs["vy"]:.2f}) m/s'
            )

    def _make_element(self, eid, rx, ry, rvx, rvy, radius, dynamic):
        el = ElementCharacteristicsStamped()
        el.header.stamp    = self.get_clock().now().to_msg()
        el.header.frame_id = 'base_link'
        el.id              = eid
        el.dynamic         = dynamic
        el.pose.position.x = rx
        el.pose.position.y = ry
        el.velocity.x      = rvx
        el.velocity.y      = rvy
        el.size.x = el.size.y = el.size.z = radius * 2.0
        return el

    def _publish(self):
        if self._agent_x is None:
            return

        out = ElementCharacteristicsArray()
        eid = 0

        for obs in self._static:
            dx, dy = obs['x'] - self._agent_x, obs['y'] - self._agent_y
            rx, ry = world_to_body(dx, dy, self._agent_yaw)
            out.elements.append(self._make_element(eid, rx, ry, 0.0, 0.0, obs['r'], False))
            eid += 1

        for name, obs in self._tracked.items():
            if obs['moving'] and not obs['live']:
                self.get_logger().warning(
                    f'{name}: no odometry yet on /model/{name}/odometry — '
                    'element withheld (is the obstacle bridge up?)',
                    throttle_duration_sec=5.0
                )
                eid += 1
                continue
            dx, dy   = obs['x'] - self._agent_x, obs['y'] - self._agent_y
            rx, ry   = world_to_body(dx, dy, self._agent_yaw)
            rvx, rvy = world_to_body(obs['vx'], obs['vy'], self._agent_yaw)
            dynamic  = math.hypot(obs['vx'], obs['vy']) > 0.05
            out.elements.append(self._make_element(eid, rx, ry, rvx, rvy, obs['r'], dynamic))
            eid += 1

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
