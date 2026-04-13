#!/usr/bin/env python3

import os
import csv
import matplotlib.pyplot as plt

plt.rcParams.update({
    'font.size': 20,
    'axes.labelsize': 22,
    'axes.titlesize': 24,
    'xtick.labelsize': 18,
    'ytick.labelsize': 18,
    'legend.fontsize': 18,
    'figure.titlesize': 26
})

import math
import numpy as np
import glob
from multiprocessing import Pool

def process_single_scenario(scenario_path):
    scenario_id = os.path.basename(scenario_path)
    csv_paths = sorted(glob.glob(os.path.join(scenario_path, '*.csv')))
    if not csv_paths:
        return False
        
    csv_files = {}
    for path in csv_paths:
        algo_name = os.path.splitext(os.path.basename(path))[0]
        if algo_name == 'javoa_ablated':
            continue
        csv_files[algo_name] = path
    
    for prefix, csv_path in csv_files.items():
        try:
            generate_plots(scenario_path, csv_path, prefix, scenario_id)
        except Exception:
            pass
            
    return True

def process_results(base_dir):
    scenarios = []
    # Iterate over all subdirectories in the base results directory
    for root, dirs, files in os.walk(base_dir):
        for dir_name in dirs:
            # Check if directory starts with 's' and isn't 's_' (scenario result folder)
            if not dir_name.startswith('s') or dir_name == 'scripts':
                continue
                
            scenario_path = os.path.join(root, dir_name)
            scenarios.append(scenario_path)
            
    if not scenarios:
        print("No scenarios found.")
        return
        
    print(f"Found {len(scenarios)} scenarios. Processing with 5 workers...")
    
    completed = 0
    with Pool(processes=5) as pool:
        for _ in pool.imap_unordered(process_single_scenario, scenarios):
            completed += 1
            print(f"\rCompleted processing {completed}/{len(scenarios)} scenarios.", end="", flush=True)
            
    print("\nAll done!")
                #         try:
                #             generate_comparison_plots(scenario_path, csv_files[a1], csv_files[a2], a1, a2, suffix=f"_{a1}_vs_{a2}")
                #         except Exception as e:
                #             print(f"Error generating comparison {a1} vs {a2} for {dir_name}: {e}")

def generate_plots(output_dir, csv_file, prefix_name, scenario_id):
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
    obstacles = [str(i) for i in range(1, 51)]
    
    has_valid_obstacles = False
    
    for obs_key in obstacles:
        prefix = f'obs_{obs_key}'
        x_col = f'{prefix}_pos_x'
        y_col = f'{prefix}_pos_y'
        z_col = f'{prefix}_pos_z'
        
        # Check if columns exist in first row keys
        if x_col not in data[0]:
            continue
            
        # Get data
        obs_x = [float(row[x_col]) for row in data]
        obs_y = [float(row[y_col]) for row in data]
        obs_z = [float(row.get(z_col, 0.0)) for row in data]

        # Check if any point is non-zero (assuming 0,0,0 means obstacle not tracked)
        is_tracked = [(ox != 0.0 or oy != 0.0 or oz != 0.0) for ox, oy, oz in zip(obs_x, obs_y, obs_z)]
        
        if not any(is_tracked):
            continue
            
        has_valid_obstacles = True
        
        # Calculate distances
        distances = []
        plot_times = []
        
        for i in range(len(obs_x)):
            if is_tracked[i]:
                dx = agent_x[i] - obs_x[i]
                dy = agent_y[i] - obs_y[i]
                dz = agent_z[i] - obs_z[i]
                dist = math.sqrt(dx*dx + dy*dy + dz*dz)
                
                # Clearance = dist - 1.5, clip to 0
                clearance = max(0.0, dist - 1.5)
                
                distances.append(clearance)
                plot_times.append(time_elapsed[i])
            else:
                distances.append(float('nan')) # Create gaps
                plot_times.append(time_elapsed[i]) # Keep time aligned
        
        plt.plot(plot_times, distances, label=f'Obstacle {obs_key}')

    if has_valid_obstacles:
        plt.title('Clearance to Obstacles over Time', fontsize=18)
        plt.xlabel('Time (s)', fontsize=16)
        plt.ylabel('Clearance (m)', fontsize=16)
        plt.xticks(fontsize=14)
        plt.yticks(fontsize=14)
        plt.legend(fontsize=12)
        plt.grid(True)
        plt.ylim(-1, 3)
        
        # Add a zero line for reference (collision threshold)
        plt.axhline(0, color='red', linestyle='--', linewidth=1, label='Collision')
        
        # Clean labels
        display_name = 'S3VO (Ablated)' if prefix_name == 'javoa_ablated' else ('S3VO' if prefix_name == 'javoa' else ('ORCA' if prefix_name == 'rvo' else prefix_name.upper()))
        plt.title(f'Clearance to All Obstacles - {display_name}', fontsize=18)
        
        plot_path = os.path.join(output_dir, f'{prefix_name}_clearance_to_obstacles.png')
        plt.tight_layout()
        plt.savefig(plot_path, bbox_inches='tight', pad_inches=0.1)
        plt.close()

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
                if f'{prefix}_pos_x' not in data[i]: continue
                
                ox = float(data[i][f'{prefix}_pos_x'])
                oy = float(data[i][f'{prefix}_pos_y'])
                oz = float(data[i].get(f'{prefix}_pos_z', 0.0))
                
                is_valid = (ox != 0.0 or oy != 0.0 or oz != 0.0)
                if is_valid:
                    dist = math.sqrt((ax-ox)**2 + (ay-oy)**2 + (az-oz)**2)
                    # Clearance = dist - 1.5, clip to 0
                    clearance = max(0.0, dist - 1.5)
                    
                    if clearance < min_dist:
                        min_dist = clearance
                    valid_obs_found = True
            
            if valid_obs_found:
                min_clearances.append(min_dist)
                min_clearance_times.append(float(data[i]['time_elapsed']))
            else:
                min_clearances.append(float('nan'))
                min_clearance_times.append(float(data[i]['time_elapsed']))
                
        plt.figure(figsize=(8, 8))
        plt.plot(min_clearance_times, min_clearances, label='Min Clearance', color='blue', linewidth=3)
        display_name = 'S3VO (Ablated)' if prefix_name == 'javoa_ablated' else ('S3VO' if prefix_name == 'javoa' else ('ORCA' if prefix_name == 'rvo' else prefix_name.upper()))
        
        # Format Scenario S083 -> Scenario 083
        formatted_scenario = f"Scenario {scenario_id.replace('s', '')}"
        
        plt.title(f'{formatted_scenario} - Minimum Clearance\n{display_name}', fontsize=24, pad=20, fontweight='bold')
        plt.xlabel('Time (s)', fontsize=22, labelpad=15)
        plt.ylabel('Clearance (m)', fontsize=22, labelpad=15)
        plt.xticks(fontsize=18)
        plt.yticks(fontsize=18)
        plt.legend(fontsize=18)
        plt.grid(True, linestyle='--', alpha=0.7)
        plt.axhline(0, color='red', linestyle='--', linewidth=2, label='Collision')
        plt.ylim(-1, 3) # Consistent scale
        
        path_min = os.path.join(output_dir, f'{prefix_name}_min_clearance.png')
        plt.tight_layout()
        plt.savefig(path_min, bbox_inches='tight', pad_inches=0.1)
        plt.close()
        # print(f"Generated plot: {path_min}")

    else:
        plt.close()
        print("No valid obstacles found to plot.")

    # --- NEW PLOTS: Velocity Components ---
    # Commented out per user request
    # try:
    #     ...
    # except Exception as e:
    #     print(f"Could not generate velocity plots: {e}")

    # --- NEW PLOT: 3D Trajectory (Static PNG) ---
    try:
        fig = plt.figure(figsize=(10, 8))
        ax = fig.add_subplot(111, projection='3d')
        
        # Agent
        ax.plot(agent_x, agent_y, agent_z, label='Agent', color='blue', linewidth=2)
        ax.scatter([agent_x[0]], [agent_y[0]], [agent_z[0]], color='green', s=50, label='Start')
        ax.scatter([agent_x[-1]], [agent_y[-1]], [agent_z[-1]], color='red', s=50, marker='x', label='End')
        
        # Obstacles
        if has_valid_obstacles:
            for obs_key in obstacles:
                prefix_obs = f'obs_{obs_key}'
                if f'{prefix_obs}_pos_x' not in data[0]: continue
                
                ox = [float(row[f'{prefix_obs}_pos_x']) for row in data]
                oy = [float(row[f'{prefix_obs}_pos_y']) for row in data]
                oz = [float(row.get(f'{prefix_obs}_pos_z', 0.0)) for row in data]
                
                is_valid = [(c_x != 0.0 or c_y != 0.0 or c_z != 0.0) for c_x, c_y, c_z in zip(ox, oy, oz)]
                if any(is_valid):
                    valid_ox = [ox[j] for j in range(len(ox)) if is_valid[j]]
                    valid_oy = [oy[j] for j in range(len(oy)) if is_valid[j]]
                    valid_oz = [oz[j] for j in range(len(oz)) if is_valid[j]]
                    # Check if obstacle is dynamic (moved more than a tiny threshold)
                    dx = max(valid_ox) - min(valid_ox)
                    dy = max(valid_oy) - min(valid_oy)
                    dz = max(valid_oz) - min(valid_oz)
                    is_dynamic = (dx > 0.01 or dy > 0.01 or dz > 0.01)
                    obs_color = 'green' if is_dynamic else 'red'
                    
                    ax.plot(valid_ox, valid_oy, valid_oz, label=f'Obstacle {obs_key}', linestyle='--', linewidth=1, color=obs_color)
                    
                    # Plot 1m radius sphere at the final position of the obstacle
                    if len(valid_ox) > 0:
                        last_x = valid_ox[-1]
                        last_y = valid_oy[-1]
                        last_z = valid_oz[-1]
                        
                        u = np.linspace(0, 2 * np.pi, 20)
                        v = np.linspace(0, np.pi, 20)
                        x = 1.0 * np.outer(np.cos(u), np.sin(v)) + last_x
                        y = 1.0 * np.outer(np.sin(u), np.sin(v)) + last_y
                        z = 1.0 * np.outer(np.ones(np.size(u)), np.cos(v)) + last_z
                        
                        
                        ax.plot_wireframe(x, y, z, color=obs_color, alpha=0.3, linewidth=0.5)
        
        display_name = 'S3VO (Ablated)' if prefix_name == 'javoa_ablated' else ('S3VO' if prefix_name == 'javoa' else ('ORCA' if prefix_name == 'rvo' else prefix_name.upper()))
        formatted_scenario = f"Scenario {scenario_id.replace('s', '')}"
        
        ax.set_title(f'{formatted_scenario} - 3D Trajectory\n{display_name}', fontsize=24, y=0.95, pad=0, fontweight='bold')
        ax.set_xlabel('X (m)', fontsize=16, labelpad=15)
        ax.set_ylabel('Y (m)', fontsize=16, labelpad=15)
        ax.set_zlabel('Z (m)', fontsize=16, labelpad=15)
        ax.tick_params(axis='both', which='major', labelsize=14, pad=5)
        
        # Determine appropriate aspect ratio based on data limits
        all_x = agent_x + valid_ox if has_valid_obstacles and 'valid_ox' in locals() else agent_x
        all_y = agent_y + valid_oy if has_valid_obstacles and 'valid_oy' in locals() else agent_y
        all_z = agent_z + valid_oz if has_valid_obstacles and 'valid_oz' in locals() else agent_z
        
        if all_x and all_y and all_z:
            max_range = np.array([max(all_x)-min(all_x), max(all_y)-min(all_y), max(all_z)-min(all_z)]).max() / 2.0
            mid_x = (max(all_x)+min(all_x)) * 0.5
            mid_y = (max(all_y)+min(all_y)) * 0.5
            mid_z = (max(all_z)+min(all_z)) * 0.5
            
            ax.set_xlim(mid_x - max_range, mid_x + max_range)
            ax.set_ylim(mid_y - max_range, mid_y + max_range)
            ax.set_zlim(mid_z - max_range, mid_z + max_range)
        
        # Legend might be too big if many obstacles, so limit or place outside
        # User requested to remove the legend entirely (Agent, Start, End)
        # if has_valid_obstacles and len(valid_ox) > 5:
        #      handles, labels = ax.get_legend_handles_labels()
        #      # Keep only Agent, Start, End
        #      ax.legend(handles[:3], labels[:3], loc='best', fontsize=14)
        # else:
        #      ax.legend(loc='best', fontsize=14)
             
        # Default isometric-ish view
        path_3d_png = os.path.join(output_dir, f'{prefix_name}_3d_trajectory_iso.png')
        plt.savefig(path_3d_png, bbox_inches='tight', pad_inches=0.1)
        
        # Set Top View (Elevation = 90, Azimuth = -90)
        ax.view_init(elev=90, azim=-90)
        # Remove Z labels for top view to prevent overlap
        ax.set_zticklabels([])
        ax.set_zlabel('')
        path_3d_top_png = os.path.join(output_dir, f'{prefix_name}_3d_trajectory_top.png')
        plt.savefig(path_3d_top_png, bbox_inches='tight', pad_inches=0.1)
        
        # Restore Z labels, remove Y labels for side view (XZ plane)
        ax.set_zlabel('Z (m)', fontsize=16, labelpad=15)
        ax.set_yticklabels([])
        ax.set_ylabel('')
        
        # Set Side View (Elevation = 0, Azimuth = -90)
        ax.view_init(elev=0, azim=-90)
        # Restore standard locators
        ax.xaxis.set_major_locator(plt.MaxNLocator(5))
        ax.yaxis.set_major_locator(plt.MaxNLocator(5))
        ax.zaxis.set_major_locator(plt.MaxNLocator(5))
        
        # Note: calling MaxNLocator brings ticks back, so remove Y ticks explicitly after
        ax.set_yticklabels([])
        
        path_3d_side_png = os.path.join(output_dir, f'{prefix_name}_3d_trajectory_side.png')
        plt.savefig(path_3d_side_png, bbox_inches='tight', pad_inches=0.1)

        plt.close()
        
    except Exception as e:
        print(f"Could not generate 3D trajectory plot: {e}")

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
        vz = float(row.get('vel_z', 0.0))
        vel_mags.append(math.sqrt(vx**2 + vy**2 + vz**2))
        
    # Min Clearance
    min_clearances = []
    obstacles = [str(i) for i in range(1, 51)]
    
    for i in range(len(data)):
        min_dist = float('inf')
        valid_found = False
        
        ax = float(data[i]['pos_x'])
        ay = float(data[i]['pos_y'])
        az = float(data[i].get('pos_z', 0.0)) # Agent Z might not be in older logs? Check headers. Orchestrator writes it.
        
        for obs_key in obstacles:
            prefix = f'obs_{obs_key}'
            if f'{prefix}_pos_x' not in data[i]: continue
            
            ox = float(data[i][f'{prefix}_pos_x'])
            oy = float(data[i][f'{prefix}_pos_y'])
            oz = float(data[i].get(f'{prefix}_pos_z', 0.0))
            is_valid = (ox != 0.0 or oy != 0.0 or oz != 0.0)
            
            if is_valid:
                dist = math.sqrt((ax-ox)**2 + (ay-oy)**2 + (az-oz)**2)
                # Clearance, clip to 0
                clr = max(0.0, dist - 1.5)
                if clr < min_dist: min_dist = clr
                valid_found = True
        
        if valid_found: min_clearances.append(min_dist)
        else: min_clearances.append(float('nan'))
        
    return times, min_clearances, vel_mags

def generate_comparison_plots(output_dir, csv_algo1, csv_algo2, name1='Algo1', name2='Algo2', suffix=''):
    t_algo1, clr_algo1, vel_algo1 = get_data_for_comparison(csv_algo1)
    t_algo2, clr_algo2, vel_algo2 = get_data_for_comparison(csv_algo2)
    
    if t_algo1 is None or t_algo2 is None:
        print("Comparison failed: Empty data")
        return

    # Use nicely capitalized labels
    label1 = 'S3VO (Ablated)' if name1 == 'javoa_ablated' else ('S3VO' if name1 == 'javoa' else name1.upper())
    label2 = 'S3VO (Ablated)' if name2 == 'javoa_ablated' else ('S3VO' if name2 == 'javoa' else name2.upper())

    # Plot 1: Min Clearance Comparison
    plt.figure(figsize=(10, 6))
    plt.plot(t_algo1, clr_algo1, label=label1, color='red', alpha=0.8, linewidth=2)
    plt.plot(t_algo2, clr_algo2, label=label2, color='blue', alpha=0.8, linewidth=2)
    plt.title('Comparison: Minimum Clearance over Time', fontsize=18)
    plt.xlabel('Time (s)', fontsize=16)
    plt.ylabel('Clearance (m)', fontsize=16)
    plt.xticks(fontsize=14)
    plt.yticks(fontsize=14)
    plt.axhline(0, color='black', linestyle='--', linewidth=1.5, label='Collision')
    plt.legend(fontsize=14)
    plt.grid(True)
    plt.ylim(-1, 3)
    
    p1 = os.path.join(output_dir, f'comparison_min_clearance{suffix}.png')
    plt.savefig(p1)
    plt.close()
    print(f"Generated comparison: {p1}")

    # Plot 2: Velocity Magnitude Comparison
    plt.figure(figsize=(10, 6))
    plt.plot(t_algo1, vel_algo1, label=label1, color='red', alpha=0.8, linewidth=2)
    plt.plot(t_algo2, vel_algo2, label=label2, color='blue', alpha=0.8, linewidth=2)
    plt.title('Comparison: Agent Velocity Magnitude over Time', fontsize=18)
    plt.xlabel('Time (s)', fontsize=16)
    plt.ylabel('Velocity (m/s)', fontsize=16)
    plt.xticks(fontsize=14)
    plt.yticks(fontsize=14)
    plt.legend(fontsize=14)
    plt.grid(True)
    
    p2 = os.path.join(output_dir, f'comparison_velocity{suffix}.png')
    plt.savefig(p2)
    plt.close()
    print(f"Generated comparison: {p2}")

if __name__ == "__main__":
    results_dir = os.path.expanduser('~/ros2_ws/src/avoa3d/results/Final_100')

    if os.path.exists(results_dir):
        process_results(results_dir)
    else:
        print(f"Results directory not found: {results_dir}")
