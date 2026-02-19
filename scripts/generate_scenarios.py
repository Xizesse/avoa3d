#!/usr/bin/env python3

import os
import random
import argparse
import math

def generate_sdf(scenario_id, seed, output_dir):
    random.seed(seed + scenario_id)  # Determinisitc seed per scenario
    
    # -------------------------------------------------------------------------
    # Configuration
    # -------------------------------------------------------------------------
    
    # Map Bounds for Obstacles (keep away from agent start if possible)
    # Agent is at -5, 0. Let's spawn obstacles in a region in front of it?
    # Or just random in the world.
    # complex0 has obstacles at (3, -3) and (3, 3).
    # Let's define a spawning area.
    MIN_X, MAX_X = -5.0, 10.0
    MIN_Y, MAX_Y = -5.0, 5.0
    MIN_Z, MAX_Z = -2.0, 2.0

    # Obstacle Parameters
    MIN_OBSTACLES = 10
    MAX_OBSTACLES = 10
    MIN_RADIUS = 0.5
    MAX_RADIUS = 0.5
    
    # Linear Velocity Parameters
    MIN_VEL = 0.0
    MAX_VEL = 2.0
    
    # -------------------------------------------------------------------------
    # Templates
    # -------------------------------------------------------------------------
    
    header = f"""<?xml version="1.0" ?>
<sdf version="1.8">
    <world name="3d_world">
        <physics name="1ms" type="ode">
            <max_step_size>0.001</max_step_size>
            <real_time_factor>1.0</real_time_factor>
            <real_time_update_rate>1000.0</real_time_update_rate>
        </physics>
        <gravity>0 0 0</gravity>

        <plugin
            filename="gz-sim-physics-system"
            name="gz::sim::v8::systems::Physics">
        </plugin>
        <plugin
            filename="libignition-gazebo-user-commands-system.so"
            name="ignition::gazebo::systems::UserCommands">
        </plugin>
        <plugin
            filename="gz-sim-scene-broadcaster-system"
            name="gz::sim::systems::SceneBroadcaster">
        </plugin>
        <plugin
            filename="gz-sim-sensors-system"
            name="gz::sim::systems::Sensors">
            <render_engine>ogre2</render_engine>
        </plugin>

        <light type="directional" name="sun"> 
            <cast_shadows>true</cast_shadows>
            <pose>0 0 10 0 0 0</pose>
            <diffuse>0.8 0.8 0.8 1</diffuse>
            <specular>0.2 0.2 0.2 1</specular>
            <attenuation>
                <range>1000</range>
                <constant>0.9</constant>
                <linear>0.01</linear>
                <quadratic>0.001</quadratic>
            </attenuation>
            <direction>-0.5 0.1 -0.9</direction>
        </light>
"""

    # Agent (Fixed at start position for now, or randomize if requested)
    agent_xml = """
        <!-- AGENTE -->
        <model name='agente' canonical_link='chassis'>
            <pose relative_to='world'>-10.0 0 0.0 0 0 0</pose>
            <link name='chassis'>
                <pose relative_to='__model__'>0.0 0 0.0 0 0 0</pose>
                <inertial>
                    <mass>1.14395</mass>
                    <inertia>
                        <ixx>0.095329</ixx>
                        <ixy>0</ixy>
                        <ixz>0</ixz>
                        <iyy>0.381317</iyy>
                        <iyz>0</iyz>
                        <izz>0.476646</izz>
                    </inertia>
                </inertial>
                <visual name='visual'>
                    <geometry>
                        <sphere>
                            <radius>0.5</radius>
                        </sphere>
                    </geometry>
                    <material>
                        <ambient>0.0 0.0 1.0 1</ambient>
                        <diffuse>0.0 0.0 1.0 1</diffuse>
                        <specular>0.0 0.0 1.0 1</specular>
                    </material>
                </visual>
                <collision name='collision'>
                    <geometry>
                        <sphere>
                            <radius>0.5</radius>
                      <sphere>
                            <radius>0.5</radius>
                        </sphere>  </sphere>
                    </geometry>
                </collision>
                <enable_gravity>false</enable_gravity>
            </link>
            
            <link name='lidar_link'>
                <pose relative_to='chassis'>0.0 0 0.1 0 0 0</pose>
                <inertial>
                    <mass>0.1</mass>
                    <inertia>
                        <ixx>0.000166667</ixx>
                        <iyy>0.000166667</iyy>
                        <izz>0.000166667</izz>
                    </inertia>
                </inertial>
                <collision name='collision'>
                    <geometry>
                        <cylinder>
                            <radius>0.05</radius>
                            <length>0.1</length>
                        </cylinder>
                    </geometry>
                </collision>
                <visual name='visual'>
                    <geometry>
                        <cylinder>
                            <radius>0.05</radius>
                            <length>0.1</length>
                        </cylinder>
                    </geometry>
                    <material>
                        <ambient>0.0 1.0 0.0 1</ambient>
                        <diffuse>0.0 1.0 0.0 1</diffuse>
                        <specular>0.0 1.0 0.0 1</specular>
                    </material>
                </visual>
                <sensor name="lidar" type="gpu_lidar">
                    <topic>scan</topic>
                    <update_rate>10</update_rate>
                    <lidar>
                        <scan>
                            <horizontal>
                                <samples>5000</samples>
                                <resolution>1</resolution>
                                <min_angle>-3.14159</min_angle>
                                <max_angle>3.14159</max_angle>
                            </horizontal>
                        </scan>
                        <range>
                            <min>0.1</min>
                            <max>10.0</max>
                            <resolution>0.01</resolution>
                        </range>
                    </lidar>
                    <visualize>true</visualize>
                    <always_on>true</always_on>
                    <update_rate>10</update_rate>
                    <pose_frame>lidar</pose_frame>
                </sensor>
                <enable_gravity>false</enable_gravity>
            </link>

            <joint name="lidar_joint" type="fixed">
                <parent>chassis</parent>
                <child>lidar_link</child>
            </joint>
            
            <plugin
                filename="ignition-gazebo-joint-state-publisher-system"
                name="ignition::gazebo::systems::JointStatePublisher">
            </plugin>
            <plugin
                filename="ignition-gazebo-odometry-publisher-system"
                name="ignition::gazebo::systems::OdometryPublisher">
                <dimensions>3</dimensions>
            </plugin>
            <plugin
                filename="ignition-gazebo-velocity-control-system"
                name="ignition::gazebo::systems::VelocityControl">
                <initial_linear>0.0 0 0</initial_linear>
                <initial_angular>0 0 0.0</initial_angular>
            </plugin> 
            <plugin
                filename="libignition-gazebo-pose-publisher-system.so"
                name="ignition::gazebo::systems::PosePublisher">
                <topic>/agente/pose</topic>
                <frame_id>agente</frame_id>
                <update_rate>30</update_rate>
            </plugin>
        </model>
"""

    footer = """
    </world>
</sdf>
"""

    # -------------------------------------------------------------------------
    # Obstacle Generation
    # -------------------------------------------------------------------------
    
    obstacles_xml = ""
    num_obstacles = random.randint(MIN_OBSTACLES, MAX_OBSTACLES)
    
    # Constants based on your requirements
    ROBOT_START_X = -10.0
    ROBOT_SPEED = 2.5  # m/s to reach goal (dist=20m) in 8 seconds
    
    for i in range(num_obstacles):
        # 1. Spawn obstacles further out so they have room to "run in"
        pos_x = random.uniform(-10.0, 10.0)
        pos_y = random.uniform(-20.0, 20.0) # Spawn specifically on the flanks
        pos_z = random.uniform(-5.0, 5.0)
        
        # 2. Pick a random "Time to Impact" (when the robot will be near this X)
        # We want obstacles to hit the path between t=2s and t=7s
        t_col = random.uniform(2.0, 7.0)
        
        # 3. Calculate where the robot will be at t_col
        target_x = ROBOT_START_X + (ROBOT_SPEED * t_col)
        target_y = 0.0
        target_z = 0.0
        
        # 4. Calculate required velocity components to reach (target) at (t_col)
        # Velocity = Distance / Time
        vel_x = (target_x - pos_x) / t_col
        vel_y = (target_y - pos_y) / t_col
        vel_z = (target_z - pos_z) / t_col
        
        # Cap the speed to your MAX_VEL if the math gets too aggressive
        current_speed = math.sqrt(vel_x**2 + vel_y**2 + vel_z**2)
        if current_speed > MAX_VEL:
            scale = MAX_VEL / current_speed
            vel_x *= scale
            vel_y *= scale
            vel_z *= scale
        
        # Size
        radius = random.uniform(MIN_RADIUS, MAX_RADIUS)
        
        obstacle_block = f"""
        <!-- OBSTACLE {i+1} -->
        <model name='obstacle_{i+1}' canonical_link='obstacle'>
            <static>false</static>
            <pose relative_to='world'>{pos_x:.2f} {pos_y:.2f} {pos_z:.2f} 0 0 0</pose>
            <link name='obstacle'>
                <pose relative_to='__model__'>0.0 0 0.0 0 0 0</pose>
                <visual name='visual'>
                    <geometry>
                        <sphere>
                            <radius>{radius:.2f}</radius> 
                        </sphere>
                    </geometry>
                    <material>
                        <ambient>1.0 0.0 0.0 1</ambient>
                        <diffuse>1.0 0.0 0.0 1</diffuse>
                        <specular>1.0 0.0 0.0 1</specular>
                    </material>
                </visual>
                <collision name='collision'>
                    <geometry>
                        <sphere>
                            <radius>{radius:.2f}</radius>
                        </sphere>
                    </geometry>
                </collision>
                <enable_gravity>false</enable_gravity>
            </link>
            
            <plugin
                filename="ignition-gazebo-velocity-control-system"
                name="ignition::gazebo::systems::VelocityControl">
                <initial_linear>{vel_x:.2f} {vel_y:.2f} {vel_z:.2f}</initial_linear>
                <initial_angular>0 0 0</initial_angular>
            </plugin> 
            
            <plugin
                filename="ignition-gazebo-odometry-publisher-system"
                name="ignition::gazebo::systems::OdometryPublisher">
                <dimensions>3</dimensions>
            </plugin>
        </model>
"""
        obstacles_xml += obstacle_block

    # -------------------------------------------------------------------------
    # File Writing
    # -------------------------------------------------------------------------
    
    full_sdf = header + agent_xml + obstacles_xml + footer
    filename = f"scenario_{scenario_id:03d}.sdf"
    filepath = os.path.join(output_dir, filename)
    
    with open(filepath, 'w') as f:
        f.write(full_sdf)
    
    return filepath


def main():
    parser = argparse.ArgumentParser(description="Generate random AVOA3D scenarios (SDF)")
    parser.add_argument('--seed', type=int, default=67 , help='Random seed')
    parser.add_argument('--count', type=int, default=100, help='Number of scenarios to generate')
    parser.add_argument('--out-dir', type=str, 
                        default=os.path.join(os.environ.get('HOME'), 'ros2_ws/src/avoa3d/scenarios'),
                        help='Output directory')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.out_dir):
        os.makedirs(args.out_dir)
        print(f"Created directory: {args.out_dir}")
        
    print(f"Generating {args.count} scenarios in {args.out_dir} with seed {args.seed}...")
    
    for i in range(args.count):
        generate_sdf(i, args.seed, args.out_dir)
        
    print("Done.")

if __name__ == '__main__':
    main()
