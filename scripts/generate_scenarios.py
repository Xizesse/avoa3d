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
    MIN_X, MAX_X = -5.0, 15.0
    MIN_Y, MAX_Y = -5.0, 5.0
    MIN_Z, MAX_Z = -2.0, 2.0

    # Obstacle Parameters
    MIN_OBSTACLES = 10
    MAX_OBSTACLES = 50
    MIN_RADIUS = 1.0
    MAX_RADIUS = 1.0
    
    # Static Wall Parameters
    MIN_WALLS = 2
    MAX_WALLS = 4
    WALL_SIZES = [4, 6, 8]
    WALL_X_RANGE = (-5.0, 8.0)
    
    # Dynamic Obstacle Parameters
    MIN_DYNAMIC_CLUSTERS = 2
    MAX_DYNAMIC_CLUSTERS = 6
    DYNAMIC_SIZES = [1, 2]
    DYNAMIC_X_RANGE = (-5.0, 10.0)
    MIN_DIST_FROM_OTHER_CLUSTERS = 3.0
    
    # Linear Velocity Parameters
    MIN_VEL = 0.0
    MAX_VEL = 4.0

    # Cluster Rotation Parameters (Limits for random orientation in radians)
    MAX_CLUSTER_ROLL = 0.0 #math.pi       # max deviation +/- (e.g., math.pi/4 for 45 deg)
    MAX_CLUSTER_PITCH = 0.0 #math.pi
    MAX_CLUSTER_YAW = 0.0 #math.pi
    
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

        <gui fullscreen="0">
            <camera name="user_camera">
                <pose>5.0 0.0 20.0 0.0 1.5708 1.5708</pose>
                <view_controller>orbit</view_controller>
            </camera>
        </gui>
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
    obstacle_count = 0
    c_idx = 0
    occupied_x_positions = []
    
    # Helper to rotate points
    def rotate_point(x, y, z, r, p, yw):
        x1 = x * math.cos(yw) - y * math.sin(yw)
        y1 = x * math.sin(yw) + y * math.cos(yw)
        z1 = z
        x2 = x1 * math.cos(p) + z1 * math.sin(p)
        y2 = y1
        z2 = -x1 * math.sin(p) + z1 * math.cos(p)
        x3 = x2
        y3 = y2 * math.cos(r) - z2 * math.sin(r)
        z3 = y2 * math.sin(r) + z2 * math.cos(r)
        return x3, y3, z3
        
    def add_cluster(cluster_size, cx, cy, cz, vx, vy, vz, is_wall=False):
        nonlocal obstacles_xml, obstacle_count, c_idx
        c_idx += 1
        
        if is_wall:
            cols = max(1, cluster_size // 2)
            rows = 2 if cluster_size >= 2 else 1
        else:
            cols = cluster_size
            rows = 1
            
        pitch_y = 2.0
        pitch_z = 2.0
        
        cluster_roll = random.uniform(-MAX_CLUSTER_ROLL, MAX_CLUSTER_ROLL)
        cluster_pitch = random.uniform(-MAX_CLUSTER_PITCH, MAX_CLUSTER_PITCH)
        cluster_yaw = random.uniform(-MAX_CLUSTER_YAW, MAX_CLUSTER_YAW)
        
        added = 0
        for r in range(rows):
            for c in range(cols):
                if added >= cluster_size: break
                obstacle_count += 1
                added += 1
                
                offset_x = 0.0
                offset_y = (c * pitch_y) - ((cols - 1) * pitch_y) / 2.0
                offset_z = (r * pitch_z) - ((rows - 1) * pitch_z) / 2.0
                
                rx, ry, rz = rotate_point(offset_x, offset_y, offset_z, cluster_roll, cluster_pitch, cluster_yaw)
                
                pos_x = cx + rx
                pos_y = cy + ry
                pos_z = cz + rz
                
                radius = MIN_RADIUS
                
                obstacle_block = f"""
        <!-- OBSTACLE {obstacle_count} (Cluster {c_idx}) -->
        <model name='obstacle_{obstacle_count}' canonical_link='obstacle'>
            <static>{"true" if is_wall else "false"}</static>
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
                <initial_linear>{vx:.2f} {vy:.2f} {vz:.2f}</initial_linear>
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

    # 1. Generate Static Walls
    # Agent is at X=-10, Goal is around X=10. Spawn walls in between.
    num_walls = random.randint(MIN_WALLS, MAX_WALLS)
    for _ in range(num_walls):
        if obstacle_count >= MAX_OBSTACLES: break
        
        cluster_size = random.choice(WALL_SIZES)
        if obstacle_count + cluster_size > MAX_OBSTACLES:
            cluster_size = MAX_OBSTACLES - obstacle_count
            if cluster_size < min(WALL_SIZES): break
            
        center_x = random.uniform(WALL_X_RANGE[0], WALL_X_RANGE[1])
        occupied_x_positions.append(center_x)
        
        center_y = random.uniform(MIN_Y, MAX_Y)
        center_z = 0.0 # user requested centered around z=0
        
        vel_x, vel_y, vel_z = 0.0, 0.0, 0.0
        
        add_cluster(cluster_size, center_x, center_y, center_z, vel_x, vel_y, vel_z, is_wall=True)
        
    # 2. Generate Dynamic Obstacles
    # Keep adding until we hit MIN_OBSTACLES, or add a few more randomly
    num_dynamic = random.randint(MIN_DYNAMIC_CLUSTERS, MAX_DYNAMIC_CLUSTERS)
    d_idx = 0
    while obstacle_count < MIN_OBSTACLES or (d_idx < num_dynamic and obstacle_count < MAX_OBSTACLES):
        d_idx += 1
        
        cluster_size = random.choice(DYNAMIC_SIZES)
        if obstacle_count + cluster_size > MAX_OBSTACLES:
            cluster_size = MAX_OBSTACLES - obstacle_count
            if cluster_size < min(DYNAMIC_SIZES): break
            
        # Find an X position away from walls and other dynamic clusters
        valid_x = False
        center_x = 0.0
        attempts = 0
        while not valid_x and attempts < 50:
            center_x = random.uniform(DYNAMIC_X_RANGE[0], DYNAMIC_X_RANGE[1])
            valid_x = True
            for ox in occupied_x_positions:
                if abs(center_x - ox) < MIN_DIST_FROM_OTHER_CLUSTERS:
                    valid_x = False
                    break
            attempts += 1
            
        if not valid_x:
            # If we repeatedly fail to find space, the map's X-axis is likely full.
            break
            
        occupied_x_positions.append(center_x)
        
        center_y = random.uniform(MIN_Y, MAX_Y)
        # Avoid spawning exactly at Y=0 so they have some distance to travel
        if abs(center_y) < 1.0: 
            center_y = 1.0 if center_y >= 0 else -1.0
            
        center_z = random.uniform(MIN_Z, MAX_Z)
        
        vel_mag = random.uniform(MIN_VEL or 0.5, MAX_VEL)
        
        # Point towards trajectory (Y=0, Z=0)
        dir_y = -center_y
        dir_z = -center_z
        
        # Add slight randomness
        dir_y += random.uniform(-0.2, 0.2)
        dir_z += random.uniform(-0.2, 0.2)
        
        magnitude = math.sqrt(dir_y**2 + dir_z**2)
        if magnitude > 0:
            dir_y /= magnitude
            dir_z /= magnitude
        else:
            dir_y, dir_z = 1.0, 0.0
            
        vel_x = 0.0  # No component in X velocity
        vel_y = dir_y * vel_mag
        vel_z = dir_z * vel_mag
        
        add_cluster(cluster_size, center_x, center_y, center_z, vel_x, vel_y, vel_z, is_wall=False)

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
    parser.add_argument('--seed', type=int, default=42 , help='Random seed')
    parser.add_argument('--count', type=int, default=100, help='Number of scenarios to generate')
    parser.add_argument('--out-dir', type=str, 
                        default=os.path.join(os.environ.get('HOME'), 'ros2_ws/src/avoa3d/scenarios_complex'),
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
