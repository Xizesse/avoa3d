#!/usr/bin/env python3

import csv
import os
import time
from datetime import datetime

import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry


class RawMetricsLogger(Node):
    def __init__(self):
        super().__init__('metrics3d_raw_logger')

        self.declare_parameter('scenario', 'default')
        self.declare_parameter('results_directory', os.path.expanduser('~/ros2_ws/src/avoa3d/results'))
        self.declare_parameter('agent_odometry_topic', '/model/agente/odometry')
        self.declare_parameter('main_obstacle_topic', '/model/obstacle/odometry')
        self.declare_parameter('obstacle_topic_prefix', '/model/obstacle_')
        self.declare_parameter('num_obstacles', 10)
        self.declare_parameter('log_rate_hz', 10.0)

        self.scenario = self.get_parameter('scenario').value
        self.results_dir = self.get_parameter('results_directory').value
        self.agent_odometry_topic = self.get_parameter('agent_odometry_topic').value
        self.main_obstacle_topic = self.get_parameter('main_obstacle_topic').value
        self.obstacle_topic_prefix = self.get_parameter('obstacle_topic_prefix').value
        self.num_obstacles = int(self.get_parameter('num_obstacles').value)
        self.log_rate_hz = float(self.get_parameter('log_rate_hz').value)

        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.scenario_dir = os.path.join(self.results_dir, f"{self.scenario}_{timestamp}")
        os.makedirs(self.scenario_dir, exist_ok=True)
        self.csv_filename = os.path.join(self.scenario_dir, 'raw_odometry.csv')

        self.start_time = None
        self.agent_odom = None
        self.obstacles = {}
        self.main_obstacle = None

        self.agent_sub = self.create_subscription(
            Odometry, self.agent_odometry_topic, self._agent_cb, 10)
        self.main_obstacle_sub = self.create_subscription(
            Odometry, self.main_obstacle_topic, self._main_obstacle_cb, 10)

        self.obstacle_subs = []
        for i in range(self.num_obstacles):
            topic = f"{self.obstacle_topic_prefix}{i}/odometry"
            self.obstacle_subs.append(
                self.create_subscription(
                    Odometry, topic, lambda msg, idx=i: self._obstacle_cb(idx, msg), 10
                )
            )

        self._init_csv()

        period = 1.0 / self.log_rate_hz if self.log_rate_hz > 0 else 0.1
        self.timer = self.create_timer(period, self._log_row)

        self.get_logger().info("📈 Raw metrics logger started")
        self.get_logger().info(f"📁 Output: {self.csv_filename}")

    def _init_csv(self):
        headers = [
            'timestamp',
            'time_elapsed',
            'agent_pos_x', 'agent_pos_y', 'agent_pos_z',
            'agent_vel_x', 'agent_vel_y', 'agent_vel_z',
        ]
        headers.extend(['obstacle_main_x', 'obstacle_main_y', 'obstacle_main_z'])
        for i in range(self.num_obstacles):
            headers.extend([f'obstacle_{i}_x', f'obstacle_{i}_y', f'obstacle_{i}_z'])

        with open(self.csv_filename, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow([f"Scenario: {self.scenario}"])
            writer.writerow(headers)

    def _agent_cb(self, msg):
        self.agent_odom = msg

    def _main_obstacle_cb(self, msg):
        self.main_obstacle = msg

    def _obstacle_cb(self, idx, msg):
        self.obstacles[idx] = msg

    def _log_row(self):
        now_sim = self.get_clock().now().nanoseconds / 1e9
        current_time = now_sim if now_sim > 0.0 else time.time()
        if self.start_time is None:
            self.start_time = current_time
        elapsed = current_time - self.start_time

        if self.agent_odom is not None:
            pos = self.agent_odom.pose.pose.position
            vel = self.agent_odom.twist.twist.linear
            agent_vals = [pos.x, pos.y, pos.z, vel.x, vel.y, vel.z]
        else:
            agent_vals = [float('nan')] * 6

        if self.main_obstacle is not None:
            op = self.main_obstacle.pose.pose.position
            main_vals = [op.x, op.y, op.z]
        else:
            main_vals = [float('nan')] * 3

        obstacle_vals = []
        for i in range(self.num_obstacles):
            if i in self.obstacles:
                op = self.obstacles[i].pose.pose.position
                obstacle_vals.extend([op.x, op.y, op.z])
            else:
                obstacle_vals.extend([float('nan')] * 3)

        row = [current_time, elapsed] + agent_vals + main_vals + obstacle_vals
        with open(self.csv_filename, 'a', newline='') as csvfile:
            csv.writer(csvfile).writerow(row)


def main():
    rclpy.init()
    node = RawMetricsLogger()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
