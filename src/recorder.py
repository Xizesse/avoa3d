#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
import os
import time
from datetime import datetime
import csv
import math
import sys

# DEBUG: Write to /tmp to verify execution
with open('/tmp/recorder_debug_log.txt', 'a') as f:
    f.write(f"Recorder script loaded at {time.time()}\n")

class DataRecorder(Node):
    def __init__(self):
        super().__init__('data_recorder')
        
        # Declare parameters
        self.declare_parameter('scenario', 'default')
        self.declare_parameter('algorithm', 'unknown')
        self.declare_parameter('results_directory', os.path.expanduser('~/ros2_ws/src/avoa3d/results'))
        
        # Topic parameters
        self.declare_parameter('topics.desired_vel', '/model/agente/desired_vel')
        self.declare_parameter('topics.cmd_vel', '/model/agente/cmd_vel')
        self.declare_parameter('topics.cmd_vel_unfiltered', '/model/agente/cmd_vel_unfiltered')
        self.declare_parameter('topics.odometry', '/model/agente/odometry')
        
        # Get parameters
        self.scenario = self.get_parameter('scenario').value
        self.algorithm = self.get_parameter('algorithm').value
        self.results_dir = self.get_parameter('results_directory').value
        
        # Get topic names
        self.desired_vel_topic = self.get_parameter('topics.desired_vel').value
        self.cmd_vel_topic = self.get_parameter('topics.cmd_vel').value
        self.cmd_vel_unfiltered_topic = self.get_parameter('topics.cmd_vel_unfiltered').value
        self.odometry_topic = self.get_parameter('topics.odometry').value
        
        # Create output directory: results/randomized/s_<id>_<algo>
        base_dir = os.path.join(self.results_dir, "randomized")
        
        try:
            scenario_id = int(self.scenario)
            folder_name = f"s_{scenario_id:03d}_{self.algorithm}"
        except ValueError:
            folder_name = f"s_{self.scenario}_{self.algorithm}"
            
        self.scenario_dir = os.path.join(base_dir, folder_name)
        
        if os.path.exists(self.scenario_dir):
            self.get_logger().warn(f"Directory {self.scenario_dir} already exists. Appending timestamp.")
            timestamp = datetime.now().strftime("%H%M%S")
            self.scenario_dir = os.path.join(base_dir, f"{folder_name}_{timestamp}")

        os.makedirs(self.scenario_dir, exist_ok=True)
        
        # Initialize storage
        self.current_data = {
            'pos_x': 0.0, 'pos_y': 0.0, 'pos_z': 0.0,
            'vel_x': 0.0, 'vel_y': 0.0, 'vel_z': 0.0,
            'cmd_vel_x': 0.0, 'cmd_vel_y': 0.0, 'cmd_vel_z': 0.0,
            'cmd_vel_raw_x': 0.0, 'cmd_vel_raw_y': 0.0, 'cmd_vel_raw_z': 0.0,
            'des_vel_x': 0.0, 'des_vel_y': 0.0, 'des_vel_z': 0.0,
        }
        
        # Obstacle storage
        self.obstacles = {} # {id: {'pos_x': ..., 'vel_x': ...}}
        
        # Flags
        self.received_odom = False
        
        # Subscribers
        self.create_subscription(Odometry, self.odometry_topic, self.odom_cb, 10)
        self.create_subscription(Twist, self.cmd_vel_topic, self.cmd_vel_cb, 10)
        self.create_subscription(Twist, self.cmd_vel_unfiltered_topic, self.cmd_vel_raw_cb, 10)
        self.create_subscription(Twist, self.desired_vel_topic, self.desired_vel_cb, 10)
        
        # Obstacle subscrbers
        self.create_subscription(Odometry, '/model/obstacle/odometry', lambda m: self.obstacle_cb('main', m), 10)
        for i in range(10):
            topic = f'/model/obstacle_{i}/odometry'
            self.create_subscription(Odometry, topic, lambda m, idx=i: self.obstacle_cb(idx, m), 10)
            
        # Logging timer (10 Hz)
        self.create_timer(0.1, self.log_data)
        
        self.start_time = time.time()
        self.csv_file = os.path.join(self.scenario_dir, 'recorder.csv')
        self.init_csv()
        
        self.get_logger().info(f"🔴 Recorder started. Saving to: {self.csv_file}")

    def init_csv(self):
        headers = [
            'timestamp', 'time_elapsed',
            'pos_x', 'pos_y', 'pos_z',
            'vel_x', 'vel_y', 'vel_z',
            'cmd_vel_x', 'cmd_vel_y', 'cmd_vel_z',
            'cmd_vel_raw_x', 'cmd_vel_raw_y', 'cmd_vel_raw_z',
            'des_vel_x', 'des_vel_y', 'des_vel_z',
        ]
        
        # Obstacles (main + 0-9)
        # We'll valid/invalid flag, pos and vel
        headers.extend(['obs_main_valid', 'obs_main_x', 'obs_main_y', 'obs_main_vx', 'obs_main_vy'])
        for i in range(10):
             headers.extend([f'obs_{i}_valid', f'obs_{i}_x', f'obs_{i}_y', f'obs_{i}_vx', f'obs_{i}_vy'])
             
        with open(self.csv_file, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(headers)
            
    def odom_cb(self, msg):
        self.current_data['pos_x'] = msg.pose.pose.position.x
        self.current_data['pos_y'] = msg.pose.pose.position.y
        self.current_data['pos_z'] = msg.pose.pose.position.z
        self.current_data['vel_x'] = msg.twist.twist.linear.x
        self.current_data['vel_y'] = msg.twist.twist.linear.y
        self.current_data['vel_z'] = msg.twist.twist.linear.z
        self.received_odom = True
        
    def cmd_vel_cb(self, msg):
        self.current_data['cmd_vel_x'] = msg.linear.x
        self.current_data['cmd_vel_y'] = msg.linear.y
        self.current_data['cmd_vel_z'] = msg.linear.z

    def cmd_vel_raw_cb(self, msg):
        self.current_data['cmd_vel_raw_x'] = msg.linear.x
        self.current_data['cmd_vel_raw_y'] = msg.linear.y
        self.current_data['cmd_vel_raw_z'] = msg.linear.z
        
    def desired_vel_cb(self, msg):
        self.current_data['des_vel_x'] = msg.linear.x
        self.current_data['des_vel_y'] = msg.linear.y
        self.current_data['des_vel_z'] = msg.linear.z
        
    def obstacle_cb(self, idx, msg):
        self.obstacles[idx] = {
            'pos_x': msg.pose.pose.position.x,
            'pos_y': msg.pose.pose.position.y,
            'vel_x': msg.twist.twist.linear.x,
            'vel_y': msg.twist.twist.linear.y,
            'time': self.get_clock().now().nanoseconds
        }
        
    def log_data(self):
        if not self.received_odom:
            return
            
        now = time.time()
        elapsed = now - self.start_time
        
        row = [
            now, elapsed,
            self.current_data['pos_x'], self.current_data['pos_y'], self.current_data['pos_z'],
            self.current_data['vel_x'], self.current_data['vel_y'], self.current_data['vel_z'],
            self.current_data['cmd_vel_x'], self.current_data['cmd_vel_y'], self.current_data['cmd_vel_z'],
            self.current_data['cmd_vel_raw_x'], self.current_data['cmd_vel_raw_y'], self.current_data['cmd_vel_raw_z'],
            self.current_data['des_vel_x'], self.current_data['des_vel_y'], self.current_data['des_vel_z'],
        ]
        
        # Check obstacles
        # Main
        if 'main' in self.obstacles:
            o = self.obstacles['main']
            row.extend([1, o['pos_x'], o['pos_y'], o['vel_x'], o['vel_y']])
        else:
            row.extend([0, 0, 0, 0, 0])
            
        # 0-9
        for i in range(10):
            if i in self.obstacles:
                o = self.obstacles[i]
                row.extend([1, o['pos_x'], o['pos_y'], o['vel_x'], o['vel_y']])
            else:
                row.extend([0, 0, 0, 0, 0])
                
        with open(self.csv_file, 'a', newline='') as f:
            csv.writer(f).writerow(row)

def main(args=None):
    rclpy.init(args=args)
    node = DataRecorder()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
