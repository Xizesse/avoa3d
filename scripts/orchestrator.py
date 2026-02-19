#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
import csv
import time
import os
import signal
import subprocess
import argparse
from datetime import datetime
import math

class ScenarioManager(Node):
    def __init__(self):
        super().__init__('scenario_manager')
        self.declare_parameter('scenario_id', 0)
        self.scenario_id = self.get_parameter('scenario_id').value
        
        self.declare_parameter('algorithm', 'javoa')
        self.algorithm = self.get_parameter('algorithm').value
        
        # Data storage
        self.data_buffer = []
        self.start_time = None
        self.is_recording = False
        
        # Subscribers
        self.create_subscription(Odometry, '/model/agente/odometry', self.odom_callback, 10)
        self.create_subscription(Twist, '/model/agente/cmd_vel', self.cmd_vel_callback, 10)
        self.create_subscription(Twist, '/model/agente/cmd_vel_unfiltered', self.cmd_vel_unfiltered_callback, 10)
        self.create_subscription(Twist, '/model/agente/desired_vel', self.desired_vel_callback, 10)
        
        # Obstacle subscribers (assuming up to 20 obstacles)
        self.obstacles = {}
        for i in range(20):
            topic = f'/model/obstacle_{i}/odometry'
            self.create_subscription(Odometry, topic, lambda msg, idx=i: self.obstacle_callback(msg, idx), 10)

        # State variables
        self.current_odom = None
        self.current_cmd_vel = None
        self.current_cmd_vel_unfiltered = None
        self.current_desired_vel = None
        
        # File setup
        self.results_dir = os.path.expanduser(f'~/ros2_ws/src/avoa3d/results/randomized/s{self.scenario_id:03d}')
        os.makedirs(self.results_dir, exist_ok=True)
        
        if self.algorithm == 'rvo':
            filename = 'rvo.csv'
        else:
            filename = 'javoa.csv'
            
        self.csv_file = os.path.join(self.results_dir, filename)
        self.init_csv()
        
        self.get_logger().info(f"Scenario Manager started for Scenario {self.scenario_id}, Algorithm: {self.algorithm}")
        self.start_time = self.get_clock().now()
        self.is_recording = True
        
        # Timer for recording at 10Hz
        self.create_timer(0.1, self.record_step)

    def init_csv(self):
        with open(self.csv_file, 'w', newline='') as f:
            writer = csv.writer(f)
            # Header
            header = ['time_elapsed', 
                      'pos_x', 'pos_y', 'pos_z', 
                      'vel_x', 'vel_y', 'vel_z',
                      'cmd_vel_x', 'cmd_vel_y', 'cmd_vel_z',
                      'cmd_vel_unfiltered_x', 'cmd_vel_unfiltered_y', 'cmd_vel_unfiltered_z',
                      'desired_vel_x', 'desired_vel_y', 'desired_vel_z']
            
            # Obstacle headers
            for i in range(20):
                header.extend([f'obs_{i}_pos_x', f'obs_{i}_pos_y', f'obs_{i}_pos_z',
                               f'obs_{i}_vel_x', f'obs_{i}_vel_y', f'obs_{i}_vel_z'])
            
            writer.writerow(header)

    def odom_callback(self, msg):
        self.current_odom = msg

    def cmd_vel_callback(self, msg):
        self.current_cmd_vel = msg
        
    def cmd_vel_unfiltered_callback(self, msg):
        self.current_cmd_vel_unfiltered = msg
        
    def desired_vel_callback(self, msg):
        self.current_desired_vel = msg

    def obstacle_callback(self, msg, idx):
        self.obstacles[idx] = msg

    def record_step(self):
        if not self.is_recording or self.current_odom is None:
            return
            
        now = self.get_clock().now()
        elapsed = (now - self.start_time).nanoseconds / 1e9
        
        row = [elapsed]
        
        # Agent Odom
        p = self.current_odom.pose.pose.position
        v = self.current_odom.twist.twist.linear
        row.extend([p.x, p.y, p.z, v.x, v.y, v.z])
        
        # Cmd Vel
        if self.current_cmd_vel:
            c = self.current_cmd_vel.linear
            row.extend([c.x, c.y, c.z])
        else:
            row.extend([0, 0, 0])
            
        # Cmd Vel Unfiltered
        if self.current_cmd_vel_unfiltered:
            u = self.current_cmd_vel_unfiltered.linear
            row.extend([u.x, u.y, u.z])
        else:
            row.extend([0, 0, 0])
            
        # Desired Vel
        if self.current_desired_vel:
            d = self.current_desired_vel.linear
            row.extend([d.x, d.y, d.z])
        else:
            row.extend([0, 0, 0])
            
        # Obstacles
        for i in range(20):
            if i in self.obstacles:
                obs = self.obstacles[i]
                op = obs.pose.pose.position
                ov = obs.twist.twist.linear
                row.extend([op.x, op.y, op.z, ov.x, ov.y, ov.z])
            else:
                row.extend([0, 0, 0, 0, 0, 0])
                
        # Append to file
        with open(self.csv_file, 'a', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(row)

def run_experiment():
    parser = argparse.ArgumentParser(description="Run AVOA3D experiments")
    parser.add_argument('--limit', type=int, help='Limit the number of scenarios to run', default=None)
    args = parser.parse_args()

    num_scenarios = 100
    if args.limit is not None:
        num_scenarios = min(num_scenarios, args.limit)
        print(f"Limiting execution to first {num_scenarios} scenarios.")

    algorithms = [
        {'name': 'javoa', 'launch_file': 'holonomic.launch.py'},
        {'name': 'rvo', 'launch_file': 'rvo.launch.py'}
    ]

    for i in range(num_scenarios):
        for algo in algorithms:
            print(f"Processing Scenario {i}, Algorithm: {algo['name']}")
            
            # Launch command
            cmd = [
                'ros2', 'launch', 'avoa3d', algo['launch_file'],
                f'scenario:=s{i}',
                'launch_bridge:=true',
                'launch_avoa3d:=true',
                'launch_obstacle_publisher:=true',
                'launch_rviz_marker:=false', # Disable GUI elements to save resources if needed
                'rviz_marker:=false',
                'publish_transforms:=true',
                 # We need to run the recorder node here?
                 # No, we are implementing the recorder IN this script?
                 # Wait, the PLAN said "Move data recording logic... into orchestrator.py".
                 # BUT, orchestrator.py runs valid ROS launch commands via SUBPROCESS.
                 # So orchestrator.py is NOT a ROS node itself in the traditional sense that it runs alongside.
                 # Actually, it CAN be a node if we user rclpy.
                 # The previous implementation was a script that launched the launch file AND also did recording?
                 # Or did it launch a separate recorder?
                 # The plan says "Expand ScenarioManager class to include Subscribers...".
                 # This implies orchestrator.py IS the node doing the recording.
                 # But it also launches the simulation?
                 # If it launches the simulation via subprocess, it needs to spin in parallel or be separate.
                 # A better pattern is: 
                 # 1. Orchestrator launches simulation (gz + algo + bridge).
                 # 2. Orchestrator spins ITS OWN node to record data.
                 # 3. When simulation finishes (timeout or whatever), Orchestrator kills simulation and stops recording.
            ]
            
            # Start simulation (non-blocking)
            sim_process = subprocess.Popen(
                cmd,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )
            
            # Start Recording Node
            rclpy.init()
            recorder = ScenarioManager()
            recorder.set_parameters([
                rclpy.parameter.Parameter('scenario_id', rclpy.Parameter.Type.INTEGER, i),
                rclpy.parameter.Parameter('algorithm', rclpy.Parameter.Type.STRING, algo['name'])
            ])
            
            # Run for a fixed duration (e.g. 20 seconds per scenario)
            try:
                start_time = time.time()
                while time.time() - start_time < 35: # 35 seconds timeout
                    rclpy.spin_once(recorder, timeout_sec=0.1)
            except KeyboardInterrupt:
                pass
            finally:
                recorder.destroy_node()
                rclpy.shutdown()
                
                # Kill simulation
                sim_process.send_signal(signal.SIGINT)
                sim_process.wait()
                # Ensure all ROS nodes are killed
                subprocess.run(['pkill', '-f', 'ros'], stdout=subprocess.DEVNULL)
                subprocess.run(['pkill', '-f', 'gz'], stdout=subprocess.DEVNULL)
                time.sleep(2) # Wait for cleanup

    # 5. Run Post-Processing
    print("\n---------------------------------------------------")
    print("Running Data Post-Processing...")
    print("---------------------------------------------------")
    post_process_script = os.path.expanduser('~/ros2_ws/src/avoa3d/scripts/data_post_processing.py')
    if os.path.exists(post_process_script):
        subprocess.run(['python3', post_process_script])
    else:
        print(f"Warning: Post-processing script not found at {post_process_script}")

    # 6. Run Analysis
    print("\n---------------------------------------------------")
    print("Running Global Analysis...")
    print("---------------------------------------------------")
    analysis_script = os.path.expanduser('~/ros2_ws/src/avoa3d/scripts/analysis.py')
    if os.path.exists(analysis_script):
        subprocess.run(['python3', analysis_script])
    else:
        print(f"Warning: Analysis script not found at {analysis_script}")

if __name__ == '__main__':
    run_experiment()
