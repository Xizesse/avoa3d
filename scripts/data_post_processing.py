#!/usr/bin/env python3

import os
import csv
import matplotlib.pyplot as plt
import math

def process_results(base_dir):
    # Iterate over all subdirectories in the base results directory
    for root, dirs, files in os.walk(base_dir):
        for dir_name in dirs:
            # Check if directory starts with 's_' (scenario result folder)
            if not dir_name.startswith('s_'):
                continue
                
            scenario_path = os.path.join(root, dir_name)
            
            # Process both CSVs if they exist
            csv_files = {
                'rvo': os.path.join(scenario_path, 'rvo.csv'),
                'javoa': os.path.join(scenario_path, 'javoa.csv')
            }
            
            # Individual plots
            for prefix, csv_path in csv_files.items():
                if os.path.exists(csv_path):
                    print(f"Processing {dir_name} / {prefix}...")
                    try:
                        generate_plots(scenario_path, csv_path, prefix)
                    except Exception as e:
                        print(f"Error processing {dir_name}/{prefix}: {e}")
                else:
                    print(f"Skipping {dir_name}/{prefix}: File not found.")
            
            # Comparative plots (if both exist)
            if os.path.exists(csv_files['rvo']) and os.path.exists(csv_files['javoa']):
                print(f"Generating comparison plots for {dir_name}...")
                try:
                    generate_comparison_plots(scenario_path, csv_files['rvo'], csv_files['javoa'])
                except Exception as e:
                    print(f"Error generating comparison for {dir_name}: {e}")

def generate_plots(output_dir, csv_file, prefix_name):
    # Load data using csv module
    data = []
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            data.append(row)
            
    if not data:
        print("CSV is empty.")
        return

    # Helper to get column as float list
    def get_col(col_name):
        return [float(row[col_name]) for row in data]

    # Extract Agent Position
    try:
        agent_x = get_col('pos_x')
        agent_y = get_col('pos_y')
        agent_z = get_col('pos_z')
        time_elapsed = get_col('time_elapsed')
    except (ValueError, KeyError) as e:
        print(f"Error parsing columns: {e}")
        return
    
    # Setup Plot
    plt.figure(figsize=(10, 6))
    
    # Obstacle keys
    obstacles = ['main'] + [str(i) for i in range(10)]
    
    has_valid_obstacles = False
    
    for obs_key in obstacles:
        prefix = f'obs_{obs_key}'
        valid_col = f'{prefix}_valid'
        x_col = f'{prefix}_x'
        y_col = f'{prefix}_y'
        z_col = f'{prefix}_z'
        
        # Check if columns exist in first row keys
        if valid_col not in data[0]:
            continue
            
        # Get data
        valid = [int(float(row[valid_col])) for row in data]
        obs_x = [float(row[x_col]) for row in data]
        obs_y = [float(row[y_col]) for row in data]
        obs_z = [float(row.get(z_col, 0.0)) for row in data] # Use .get for backward compatibility (default 0.0)

        # Check if any valid
        if not any(v == 1 for v in valid):
            continue
            
        has_valid_obstacles = True
        
        # Calculate distances
        distances = []
        plot_times = []
        
        for i in range(len(valid)):
            if valid[i] == 1:
                dx = agent_x[i] - obs_x[i]
                dy = agent_y[i] - obs_y[i]
                dz = agent_z[i] - obs_z[i]
                dist = math.sqrt(dx*dx + dy*dy + dz*dz)
                
                # Clearance = dist - (agent_radius + obstacle_radius)
                # agent_radius = 0.5, obstacle_radius = 0.5
                clearance = dist - 1.0
                
                distances.append(clearance)
                plot_times.append(time_elapsed[i])
            else:
                distances.append(float('nan')) # Create gaps
                plot_times.append(time_elapsed[i]) # Keep time aligned
        
        plt.plot(plot_times, distances, label=f'Obstacle {obs_key}')

    if has_valid_obstacles:
        plt.title('Clearance to Obstacles over Time')
        plt.xlabel('Time (s)')
        plt.ylabel('Clearance (m)')
        plt.legend()
        plt.grid(True)
        plt.ylim(-1, 3)
        
        # Add a zero line for reference (collision threshold)
        plt.axhline(0, color='red', linestyle='--', linewidth=1, label='Collision')
        
        plot_path = os.path.join(output_dir, f'{prefix_name}_clearance_to_obstacles.png')
        plt.savefig(plot_path)
        plt.close()
        print(f"Generated plot: {plot_path}")

        # --- NEW PLOT: Minimum Clearance to Any Obstacle ---
        # We need to re-calculate distances per timestamp across all obstacles
        # To do this efficiently, let's process row by row again or transpose our data structure
        
        min_clearances = []
        min_clearance_times = []
        
        num_rows = len(data)
        
        for i in range(num_rows):
            min_dist = float('inf')
            valid_obs_found = False
            
            # Agent pos at this step
            ax = agent_x[i]
            ay = agent_y[i]
            az = agent_z[i]
            
            for obs_key in obstacles:
                prefix = f'obs_{obs_key}'
                if f'{prefix}_valid' not in data[i]: continue
                
                is_valid = int(float(data[i][f'{prefix}_valid']))
                if is_valid == 1:
                    ox = float(data[i][f'{prefix}_x'])
                    oy = float(data[i][f'{prefix}_y'])
                    oz = float(data[i].get(f'{prefix}_z', 0.0))
                    
                    dist = math.sqrt((ax-ox)**2 + (ay-oy)**2 + (az-oz)**2)
                    # Clearance = dist - 1.0
                    clearance = dist - 1.0
                    
                    if clearance < min_dist:
                        min_dist = clearance
                    valid_obs_found = True
            
            if valid_obs_found:
                min_clearances.append(min_dist)
                min_clearance_times.append(float(data[i]['time_elapsed']))
            else:
                min_clearances.append(float('nan'))
                min_clearance_times.append(float(data[i]['time_elapsed']))
                
        plt.figure(figsize=(10, 6))
        plt.plot(min_clearance_times, min_clearances, label='Min Clearance', color='blue')
        plt.title('Minimum Clearance to Any Obstacle over Time')
        plt.xlabel('Time (s)')
        plt.ylabel('Clearance (m)')
        plt.legend()
        plt.grid(True)
        plt.axhline(0, color='red', linestyle='--', linewidth=1, label='Collision')
        plt.ylim(-1, 3) # Consistent scale
        
        path_min = os.path.join(output_dir, f'{prefix_name}_min_clearance.png')
        plt.savefig(path_min)
        plt.close()
        print(f"Generated plot: {path_min}")

    else:
        plt.close()
        print("No valid obstacles found to plot.")

    # --- NEW PLOTS: Velocity Components ---
    # We need: cmd_vel, cmd_vel_unfiltered, desired_vel
    # Components: x, y, z
    
    # Check if columns exist
    try:
        # Cmd Vel (Filtered)
        cmd_vx = get_col('cmd_vel_x')
        cmd_vy = get_col('cmd_vel_y')
        cmd_vz = get_col('cmd_vel_z')
        
        # Unfiltered
        raw_vx = get_col('cmd_vel_unfiltered_x')
        raw_vy = get_col('cmd_vel_unfiltered_y')
        raw_vz = get_col('cmd_vel_unfiltered_z')
        
        # Desired
        des_vx = get_col('des_vel_x')
        des_vy = get_col('des_vel_y')
        des_vz = get_col('des_vel_z')
        
        time_elapsed = get_col('time_elapsed')
        
        # Helper for plotting component
        def plot_component(component_name, c_cmd, c_raw, c_des, filename):
            plt.figure(figsize=(10, 6))
            plt.plot(time_elapsed, c_des, label='Desired Vel', linestyle='--', alpha=0.7)
            plt.plot(time_elapsed, c_raw, label='Cmd Vel Unfiltered', alpha=0.6)
            plt.plot(time_elapsed, c_cmd, label='Cmd Vel (Filtered)', linewidth=2)
            
            plt.title(f'Velocity {component_name} Component over Time')
            plt.xlabel('Time (s)')
            plt.ylabel('Velocity (m/s)')
            plt.legend()
            plt.grid(True)
            
            p = os.path.join(output_dir, filename)
            plt.savefig(p)
            plt.close()
            print(f"Generated plot: {p}")
            
        plot_component('X', cmd_vx, raw_vx, des_vx, f'{prefix_name}_velocity_x.png')
        plot_component('Y', cmd_vy, raw_vy, des_vy, f'{prefix_name}_velocity_y.png')
        plot_component('Z', cmd_vz, raw_vz, des_vz, f'{prefix_name}_velocity_z.png')
        
    except Exception as e:
        print(f"Could not generate velocity plots: {e}")

def get_data_for_comparison(csv_file):
    # Helper to extract relevant data for comparison headers
    # Returns: time_elapsed, min_clearance, velocity_magnitude
    data = []
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader: data.append(row)
        
    if not data: return None, None, None
    
    times = [float(row['time_elapsed']) for row in data]
    
    # Velocity Mag (vel_x, vel_y, vel_z)
    vel_mags = []
    for row in data:
        vx = float(row['vel_x'])
        vy = float(row['vel_y'])
        vz = float(row['vel_z'])
        vel_mags.append(math.sqrt(vx**2 + vy**2 + vz**2))
        
    # Min Clearance
    min_clearances = []
    obstacles = ['main'] + [str(i) for i in range(10)]
    
    for i in range(len(data)):
        min_dist = float('inf')
        valid_found = False
        
        ax = float(data[i]['pos_x'])
        ay = float(data[i]['pos_y'])
        az = float(data[i].get('pos_z', 0.0)) # Agent Z might not be in older logs? Check headers. Orchestrator writes it.
        
        for obs_key in obstacles:
            prefix = f'obs_{obs_key}'
            if f'{prefix}_valid' not in data[i]: continue
            if int(float(data[i][f'{prefix}_valid'])) == 1:
                ox = float(data[i][f'{prefix}_x'])
                oy = float(data[i][f'{prefix}_y'])
                oz = float(data[i].get(f'{prefix}_z', 0.0))
                dist = math.sqrt((ax-ox)**2 + (ay-oy)**2 + (az-oz)**2)
                # Clearance
                clr = dist - 1.0
                if clr < min_dist: min_dist = clr
                valid_found = True
        
        if valid_found: min_clearances.append(min_dist)
        else: min_clearances.append(float('nan'))
        
    return times, min_clearances, vel_mags

def generate_comparison_plots(output_dir, rvo_csv, javoa_csv):
    t_rvo, clr_rvo, vel_rvo = get_data_for_comparison(rvo_csv)
    t_jav, clr_jav, vel_jav = get_data_for_comparison(javoa_csv)
    
    if t_rvo is None or t_jav is None:
        print("Comparison failed: Empty data")
        return

    # Plot 1: Min Clearance Comparison
    plt.figure(figsize=(10, 6))
    plt.plot(t_rvo, clr_rvo, label='RVO', color='red', alpha=0.8)
    plt.plot(t_jav, clr_jav, label='Javoa', color='blue', alpha=0.8)
    plt.title('Comparison: Minimum Clearance over Time')
    plt.xlabel('Time (s)')
    plt.ylabel('Clearance (m)')
    plt.axhline(0, color='black', linestyle='--', linewidth=1, label='Collision')
    plt.legend()
    plt.grid(True)
    plt.ylim(-1, 3)
    
    p1 = os.path.join(output_dir, 'comparison_min_clearance.png')
    plt.savefig(p1)
    plt.close()
    print(f"Generated comparison: {p1}")

    # Plot 2: Velocity Magnitude Comparison
    plt.figure(figsize=(10, 6))
    plt.plot(t_rvo, vel_rvo, label='RVO', color='red', alpha=0.8)
    plt.plot(t_jav, vel_jav, label='Javoa', color='blue', alpha=0.8)
    plt.title('Comparison: Agent Velocity Magnitude over Time')
    plt.xlabel('Time (s)')
    plt.ylabel('Velocity (m/s)')
    plt.legend()
    plt.grid(True)
    
    p2 = os.path.join(output_dir, 'comparison_velocity.png')
    plt.savefig(p2)
    plt.close()
    print(f"Generated comparison: {p2}")

if __name__ == "__main__":
    results_dir = os.path.expanduser('~/ros2_ws/src/avoa3d/results/randomized')
    if os.path.exists(results_dir):
        process_results(results_dir)
    else:
        print(f"Results directory not found: {results_dir}")
