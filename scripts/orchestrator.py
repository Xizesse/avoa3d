#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
import subprocess
import time
import os
import signal
import sys

class ScenarioManager(Node):
    def __init__(self, scenario_index):
        super().__init__('scenario_manager')
        self.target_goal = [10.0, 0.0, 0.0]  # Matches your static goal in launch
        self.threshold = 1.0  # 1.0 meter margin
        self.reached = False
        self.start_time = time.time()
        self.timeout = 60.0 * 5 # 5 minutes timeout
        
        self.sub = self.create_subscription(
            Odometry, 
            '/model/agente/odometry', 
            self.odom_callback, 
            10)
        
        self.get_logger().info(f"Monitor started for Scenario {scenario_index}")

    def odom_callback(self, msg):
        pos = msg.pose.pose.position
        dist = ((pos.x - self.target_goal[0])**2 + 
                (pos.y - self.target_goal[1])**2 + 
                (pos.z - self.target_goal[2])**2)**0.5
        
        if dist < self.threshold:
            self.get_logger().info(f"Goal Reached! Distance: {dist:.4f}m")
            self.reached = True
            
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
            monitor = ScenarioManager(i)
            
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

if __name__ == '__main__':
    run_experiment()