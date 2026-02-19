#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Twist
import subprocess
import time
import os
import signal
import sys
import csv
from datetime import datetime

class ScenarioManager(Node):
    def __init__(self, scenario_index, algorithm_name):
        super().__init__('scenario_manager')
        self.target_goal = [10.0, 0.0, 0.0]  # Matches your static goal in launch
        self.threshold = 1.0  # 1.0 meter margin
        self.reached = False
        self.start_time = time.time()
        self.timeout = 60.0 * 5 # 5 minutes timeout
        
        # Recorder Setup
        self.scenario = scenario_index
        self.algorithm = algorithm_name.replace('.launch.py', '')
        
        # Rename holonomic -> javoa
        if 'holonomic' in self.algorithm:
            self.algorithm = 'javoa'
            
        self.results_dir = os.path.expanduser('~/ros2_ws/src/avoa3d/results')
        
        # Create output directory: results/randomized/s_<id>
        base_dir = os.path.join(self.results_dir, "randomized")
        folder_name = f"s_{self.scenario:03d}"
        self.scenario_dir = os.path.join(base_dir, folder_name)
        
        # Overwrite logic: Just make directory
        if os.path.exists(self.scenario_dir):
            self.get_logger().warn(f"Directory {self.scenario_dir} exists. Files will be overwritten/added.")

        os.makedirs(self.scenario_dir, exist_ok=True)
        
        # Initialize storage
        self.current_data = {
            'pos_x': 0.0, 'pos_y': 0.0, 'pos_z': 0.0,
            'vel_x': 0.0, 'vel_y': 0.0, 'vel_z': 0.0,
            'cmd_vel_x': 0.0, 'cmd_vel_y': 0.0, 'cmd_vel_z': 0.0,
            'cmd_vel_raw_x': 0.0, 'cmd_vel_raw_y': 0.0, 'cmd_vel_raw_z': 0.0,
            'des_vel_x': 0.0, 'des_vel_y': 0.0, 'des_vel_z': 0.0,
        }
        self.obstacles = {} 
        self.received_odom = False
        
        # CSV Init
        # javoa -> javoa.csv
        # others -> recorder.csv
        if self.algorithm == 'javoa':
            csv_filename = 'javoa.csv'
        else:
            csv_filename = 'rvo.csv'
            
        self.csv_file = os.path.join(self.scenario_dir, csv_filename)
        self.init_csv()

        self.sub = self.create_subscription(
            Odometry, 
            '/model/agente/odometry', 
            self.odom_callback, 
            10)
            
        self.create_subscription(Twist, '/model/agente/cmd_vel', self.cmd_vel_cb, 10)
        self.create_subscription(Twist, '/model/agente/cmd_vel_unfiltered', self.cmd_vel_raw_cb, 10)
        self.create_subscription(Twist, '/model/agente/desired_vel', self.desired_vel_cb, 10)
        
        # Obstacle subscrbers
        self.create_subscription(Odometry, '/model/obstacle/odometry', lambda m: self.obstacle_cb('main', m), 10)
        for i in range(10):
            topic = f'/model/obstacle_{i}/odometry'
            self.create_subscription(Odometry, topic, lambda m, idx=i: self.obstacle_cb(idx, m), 10)
            
        # Logging timer (10 Hz)
        self.create_timer(0.1, self.log_data)
        
        self.get_logger().info(f"Monitor started for Scenario {scenario_index} | Algo: {self.algorithm}")
        self.get_logger().info(f"Recording to {self.csv_file}")

    def odom_callback(self, msg):
        # Monitor Goal
        pos = msg.pose.pose.position
        dist = ((pos.x - self.target_goal[0])**2 + 
                (pos.y - self.target_goal[1])**2 + 
                (pos.z - self.target_goal[2])**2)**0.5
        
        if dist < self.threshold:
            self.get_logger().info(f"Goal Reached! Distance: {dist:.4f}m")
            self.reached = True
            
        # Update Recorder Data
        self.current_data['pos_x'] = msg.pose.pose.position.x
        self.current_data['pos_y'] = msg.pose.pose.position.y
        self.current_data['pos_z'] = msg.pose.pose.position.z
        self.current_data['vel_x'] = msg.twist.twist.linear.x
        self.current_data['vel_y'] = msg.twist.twist.linear.y
        self.current_data['vel_z'] = msg.twist.twist.linear.z
        self.received_odom = True

    def init_csv(self):
        headers = [
            'timestamp', 'time_elapsed',
            'pos_x', 'pos_y', 'pos_z',
            'vel_x', 'vel_y', 'vel_z',
            'cmd_vel_x', 'cmd_vel_y', 'cmd_vel_z',
            'cmd_vel_unfiltered_x', 'cmd_vel_unfiltered_y', 'cmd_vel_unfiltered_z',
            'des_vel_x', 'des_vel_y', 'des_vel_z',
        ]
        
        # Obstacles (main + 0-9)
        headers.extend(['obs_main_valid', 'obs_main_x', 'obs_main_y', 'obs_main_z', 'obs_main_vx', 'obs_main_vy', 'obs_main_vz'])
        for i in range(10):
             headers.extend([f'obs_{i}_valid', f'obs_{i}_x', f'obs_{i}_y', f'obs_{i}_z', f'obs_{i}_vx', f'obs_{i}_vy', f'obs_{i}_vz'])
             
        with open(self.csv_file, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(headers)
            
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
            'pos_z': msg.pose.pose.position.z,
            'vel_x': msg.twist.twist.linear.x,
            'vel_y': msg.twist.twist.linear.y,
            'vel_z': msg.twist.twist.linear.z,
            'last_seen': time.time(),
            'valid': True
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
        
        # Check obstacles helper
        def get_obs_data(idx):
            if idx in self.obstacles:
                o = self.obstacles[idx]
                # Optional: Check if data is stale? e.g. > 1.0s old -> invalid?
                if (now - o['last_seen']) < 2.0:
                    return [1, o['pos_x'], o['pos_y'], o['pos_z'], o['vel_x'], o['vel_y'], o['vel_z']]
            return [0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]

        # Main obst
        row.extend(get_obs_data('main'))
            
        # 0-9 obst
        for i in range(10):
            row.extend(get_obs_data(i))
                
        with open(self.csv_file, 'a', newline='') as f:
            csv.writer(f).writerow(row)
            
    def check_timeout(self):
        if time.time() - self.start_time > self.timeout:
            self.get_logger().warn("Scenario timed out!")
            return True
        return False

def run_experiment():
    # Initialize ROS 2 context once
    rclpy.init()

    # 1. Get list of scenarios
    scenario_dir = os.path.expanduser('~/ros2_ws/src/avoa3d/scenarios')
    
    # Check if directory exists
    if not os.path.exists(scenario_dir):
        print(f"Error: Scenario directory not found at {scenario_dir}")
        return

    # Filter and sort sdf files
    scenario_files = [f for f in os.listdir(scenario_dir) if f.endswith('.sdf')]
    scenario_files.sort() # Ensure consistent order
    num_scenarios = len(scenario_files)
    
    print(f"Found {num_scenarios} scenarios.")
    
    algorithms = ["rvo.launch.py", "holonomic.launch.py"]

    import argparse
    parser = argparse.ArgumentParser(description="Run AVOA3D experiments")
    parser.add_argument('--limit', type=int, default=None, help='Limit number of scenarios to run')
    args = parser.parse_args(args=None if sys.argv[0] == __file__ else sys.argv[1:])

    # Apply limit
    if args.limit is not None and args.limit < num_scenarios:
        num_scenarios = args.limit
        print(f"Limiting execution to first {num_scenarios} scenarios.")

    for i in range(num_scenarios):
        for algo in algorithms:
            print(f"\n========================================")
            print(f"--- SCENARIO {i} | ALGORITHM: {algo} ---")
            print(f"========================================")
            
            # 2. Launch the ROS2 process
            # Using setsid to create a new process group so we can kill the whole tree later
            cmd = ["ros2", "launch", "avoa3d", algo, f"scenario:={i}"]
            
            
            # Open /dev/null to suppress output if desired, or let it print to screen
            # For now, let's keep it visible but you might want to redirect stdout/stderr
            process = subprocess.Popen(cmd, preexec_fn=os.setsid)
    
            # 3. Start Monitoring Node
            # We need a fresh node for each iteration to reset state cleanly
            monitor = ScenarioManager(i, algo)
            
            try:
                while rclpy.ok() and not monitor.reached:
                    rclpy.spin_once(monitor, timeout_sec=0.5)
                    if monitor.check_timeout():
                        break
                        
            except KeyboardInterrupt:
                print("\nCaught KeyboardInterrupt, stopping experiment...")
                # Kill the current process before exiting
                os.killpg(os.getpgid(process.pid), signal.SIGINT)
                process.wait()
                # If user interrupts, we probably want to stop the whole experiment, not just this algo
                return 
                
            finally:
                # 4. Cleanup current scenario
                print(f"Terminating {algo} for Scenario {i}...")
                
                # Send SIGINT to the process group to ensure all child processes (Gazebo, etc.) are killed
                os.killpg(os.getpgid(process.pid), signal.SIGINT) 
                
                # Wait for the process to exit
                try:
                    process.wait(timeout=10)
                except subprocess.TimeoutExpired:
                     print("Force killing process...")
                     os.killpg(os.getpgid(process.pid), signal.SIGKILL)
                     process.wait()
    
                monitor.destroy_node()
                
                # Brief sleep to let Gazebo/Nodes release ports/memory completely
                print("Waiting for cleanup...")
                time.sleep(5) 

    # Shutdown ROS 2 context
    if rclpy.ok():
        rclpy.shutdown()
        
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