#!/usr/bin/env python3

import os
import csv
import matplotlib.pyplot as plt

plt.rcParams.update({
    'font.size': 22,
    'axes.labelsize': 24,
    'axes.titlesize': 26,
    'xtick.labelsize': 20,
    'ytick.labelsize': 20,
    'legend.fontsize': 20,
    'figure.titlesize': 28,
    'lines.linewidth': 2.0,
    'axes.linewidth': 1.5
})

import math
import numpy as np

def analyze_results(base_dir):
    analysis_dir = os.path.join(base_dir, 'analysis')
    os.makedirs(analysis_dir, exist_ok=True)
    
    scenarios_processed = 0
    
    import glob
    subdirs = sorted([d for d in os.listdir(base_dir) if d.startswith('s') and d != 'scripts' and os.path.isdir(os.path.join(base_dir, d))])
    
    # 1. Discover all algorithm names from the first valid scenario folder
    algos = []
    for dir_name in subdirs:
        scenario_path = os.path.join(base_dir, dir_name)
        csv_files = glob.glob(os.path.join(scenario_path, '*.csv'))
        if csv_files:
            algos = [os.path.splitext(os.path.basename(f))[0] for f in csv_files]
            algos.sort()
            break
            
    if not algos:
        print("No CSV files found in any scenario directories.")
        return
        
    print(f"Discovered algorithms: {algos}")

    # Data storage
    data = {
        'time_to_goal': {algo: {'val': [], 'id': []} for algo in algos},
        'min_clearance': {algo: {'val': [], 'id': []} for algo in algos},
        'distance_traveled': {algo: {'val': [], 'id': []} for algo in algos},
        'smoothness': {algo: {'val': [], 'id': []} for algo in algos},
        'goal': {algo: {'val': [], 'id': []} for algo in algos}
    }
    
    success_counts = {algo: 0 for algo in algos}
    collision_counts = {algo: 0 for algo in algos}
    
    for dir_name in subdirs:
        scenario_path = os.path.join(base_dir, dir_name)
        
        # Check if all discovered algorithms have a CSV in this scenario
        missing = False
        for algo in algos:
            if not os.path.exists(os.path.join(scenario_path, f'{algo}.csv')):
                missing = True
                break
        
        if missing and len(algos) > 1:
            continue
            
        print(f"Processing {dir_name}...")
        
        try:
            scenario_results = {}
            for algo in algos:
                csv_path = os.path.join(scenario_path, f'{algo}.csv')
                if not os.path.exists(csv_path): continue
                t, c, d, s, g = process_single_csv(csv_path)
                scenario_results[algo] = {
                    'time': t, 'clearance': c, 'dist': d, 'smoothness': s, 'goal': g
                }
            
            # Check valid outputs
            if all(res['time'] is not None for res in scenario_results.values()):
                scenarios_processed += 1
                
                # Append all metrics unconditionally
                for algo, res in scenario_results.items():
                    data['min_clearance'][algo]['val'].append(res['clearance'])
                    data['min_clearance'][algo]['id'].append(dir_name)
                    data['time_to_goal'][algo]['val'].append(res['time'])
                    data['time_to_goal'][algo]['id'].append(dir_name)
                    data['distance_traveled'][algo]['val'].append(res['dist'])
                    data['distance_traveled'][algo]['id'].append(dir_name)
                    data['smoothness'][algo]['val'].append(res['smoothness'])
                    data['smoothness'][algo]['id'].append(dir_name)
                    data['goal'][algo]['val'].append(res['goal'])
                    data['goal'][algo]['id'].append(dir_name)
                    
                    if res['goal']:
                        success_counts[algo] += 1
                    if res['clearance'] <= 0.0:
                        collision_counts[algo] += 1
                
        except Exception as e:
            print(f"Error analyzing {dir_name}: {e}")
            
    if scenarios_processed == 0:
        print("No valid scenarios found with data for all algorithms.")
        return

    print(f"\nAnalysis complete. Processed {scenarios_processed} scenarios.")
    
    pairs_to_plot = [('javoa', 'javoa_ablated'), ('javoa', 'rvo')]
    for a1, a2 in pairs_to_plot:
        if a1 not in algos or a2 not in algos:
            continue
        
        print(f"Generating static matplotlib plots for {a1} vs {a2}...")
        suffix = f"_{a1}_vs_{a2}"
        
        # Helper to subset data based on both algorithms reaching the goal
        def subset(metric_name, require_both_goal=True):
            sub_d = {a1: {'val': [], 'id': []}, a2: {'val': [], 'id': []}}
            
            ids_a1 = data[metric_name][a1]['id']
            vals_a1 = data[metric_name][a1]['val']
            goals_a1 = data['goal'][a1]['val']
            
            vals_a2 = data[metric_name][a2]['val']
            goals_a2 = data['goal'][a2]['val']
            
            for i in range(len(ids_a1)):
                if not require_both_goal or (goals_a1[i] and goals_a2[i]):
                    if not math.isnan(vals_a1[i]) and not math.isnan(vals_a2[i]):
                        sub_d[a1]['val'].append(vals_a1[i])
                        sub_d[a1]['id'].append(ids_a1[i])
                        sub_d[a2]['val'].append(vals_a2[i])
                        sub_d[a2]['id'].append(ids_a1[i])
            return sub_d

        # Helper for success/collision counts which are simple dicts
        def subset_counts(counts_dict):
            return {k: counts_dict[k] for k in (a1, a2) if k in counts_dict}
            
        generate_boxplot(
            subset('time_to_goal', require_both_goal=True), 
            'Time to Reach Goal Distribution', 
            'Time (s)', 
            os.path.join(analysis_dir, f'time_to_goal_boxplot{suffix}.png')
        )
        
        generate_boxplot(
            subset('min_clearance', require_both_goal=False), 
            'Minimum Clearance Distribution', 
            'Clearance (m)', 
            os.path.join(analysis_dir, f'min_clearance_boxplot{suffix}.png')
        )

        generate_boxplot(
            subset('distance_traveled', require_both_goal=True), 
            'Distance Traveled Distribution', 
            'Distance (m)', 
            os.path.join(analysis_dir, f'distance_traveled_boxplot{suffix}.png')
        )

        generate_boxplot(
            subset('smoothness', require_both_goal=True), 
            'Smoothness Distribution', 
            'Smoothness ($m/s^2$)', 
            os.path.join(analysis_dir, f'smoothness_boxplot{suffix}.png')
        )

        generate_success_barplot(
            subset_counts(success_counts),
            scenarios_processed,
            'Goal Reach Rate ',
            'Number of Scenarios',
            os.path.join(analysis_dir, f'goal_reached_barplot{suffix}.png')
        )

        generate_success_barplot(
            subset_counts(collision_counts),
            scenarios_processed,
            'Collision Rate (Clearance <= 0)',
            'Number of Scenarios',
            os.path.join(analysis_dir, f'collision_barplot{suffix}.png'),
            color_override='lightcoral'
        )

def process_single_csv(csv_file):
    rows = []
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(row)
            
    if not rows:
        return None, None, None, None, False
        
    last_row = rows[-1]
    last_x = float(last_row['pos_x'])
    last_y = float(last_row['pos_y'])
    last_z = float(last_row.get('pos_z', 0.0))
    dist_to_goal = math.sqrt((last_x - 10.0)**2 + last_y**2 + last_z**2)
    goal_reached = dist_to_goal < 5.0
        
    # 1. Time to Goal = Max time_elapsed
    # Assuming the run ends when goal is reached or timeout. 
    # If it was a success, the last time is the time to goal.
    # Note: We don't strictly check if "success" flag exists, but max time is a good proxy for duration.
    # Using float() conversion safety
    times = [float(r['time_elapsed']) for r in rows]
    time_to_goal = max(times)
    
    # Filter out runs that timed out (even if within the goal radius at the last second)
    if time_to_goal >= 29.5:
        goal_reached = False
        time_to_goal = float('nan')
    
    # 2. Global Minimum Clearance
    # We need to calculate min clearance for every row, then find the min of those mins.
    # Reuse logic from data_post_processing
    min_clearance_global = float('inf')
    
    obstacles = [str(i) for i in range(1, 51)]
    
    for row in rows:
        current_step_min = float('inf')
        valid_found = False
        
        ax = float(row['pos_x'])
        ay = float(row['pos_y'])
        az = float(row.get('pos_z', 0.0))
        
        for obs_key in obstacles:
            prefix = f'obs_{obs_key}'
            if f'{prefix}_pos_x' not in row: continue
            
            ox = float(row[f'{prefix}_pos_x'])
            oy = float(row[f'{prefix}_pos_y'])
            oz = float(row.get(f'{prefix}_pos_z', 0.0))
            is_valid = (ox != 0.0 or oy != 0.0 or oz != 0.0)
            
            if is_valid:
                dist = math.sqrt((ax-ox)**2 + (ay-oy)**2 + (az-oz)**2)
                clr = max(0.0, dist - 1.5)
                
                if clr < current_step_min:
                    current_step_min = clr
                valid_found = True
        
        if valid_found:
            if current_step_min < min_clearance_global:
                min_clearance_global = current_step_min
                
    if min_clearance_global == float('inf'):
        min_clearance_global = float('nan') # Should not happen if valid valid obstacles exist
        
    # 3. Distance Traveled
    distance_traveled = 0.0
    for i in range(1, len(rows)):
        prev = rows[i-1]
        curr = rows[i]
        dx = float(curr['pos_x']) - float(prev['pos_x'])
        dy = float(curr['pos_y']) - float(prev['pos_y'])
        dz = float(curr.get('pos_z', 0.0)) - float(prev.get('pos_z', 0.0))
        distance_traveled += math.sqrt(dx*dx + dy*dy + dz*dz)

    # 4. Smoothness (m_vsm)
    smoothness = 0.0
    valid_steps = 0
    for i in range(1, len(rows)):
        prev = rows[i-1]
        curr = rows[i]
        
        t_prev = float(prev['time_elapsed'])
        t_curr = float(curr['time_elapsed'])
        dt = t_curr - t_prev
        
        if dt > 0:
            vx_prev = float(prev['vel_x'])
            vy_prev = float(prev['vel_y'])
            vz_prev = float(prev.get('vel_z', 0.0))
            vx_curr = float(curr['vel_x'])
            vy_curr = float(curr['vel_y'])
            vz_curr = float(curr.get('vel_z', 0.0))
            
            dvx = vx_curr - vx_prev
            dvy = vy_curr - vy_prev
            dvz = vz_curr - vz_prev
            
            dv_mag = math.sqrt(dvx*dvx + dvy*dvy + dvz*dvz)
            smoothness += dv_mag / dt
            valid_steps += 1
            
    if valid_steps > 0:
        smoothness /= valid_steps
    else:
        smoothness = float('nan')

    return time_to_goal, min_clearance_global, distance_traveled, smoothness, goal_reached

def generate_boxplot(data_dict, title, ylabel, output_path):
    plt.figure(figsize=(9, 7))
    
    data_list = []
    labels = []
    
    # Sort algorithms for consistent plotting (case-insensitive for nice display, e.g. javoa then rvo)
    algos = sorted(list(data_dict.keys()), key=lambda x: x.lower())
    
    # Generic distinct colors
    color_palette = ['lightblue', 'lightcoral', 'lightgreen', 'wheat', 'plum', 'lightgray', 'khaki']
    colors = []
    
    for i, algo in enumerate(algos):
        vals = data_dict[algo]['val']
        if vals:
            data_list.append(vals)
            # Make S3VO instead of javoa dynamically if named exactly javoa
            display_name = 'S3VO (Ablated)' if algo == 'javoa_ablated' else ('S3VO' if algo == 'javoa' else ('ORCA' if algo == 'rvo' else algo.upper()))
            labels.append(display_name)
            colors.append(color_palette[i % len(color_palette)])
            
    print(f"\n--- Quartiles for {title} ---")
    for algo in algos:
        vals = data_dict[algo]['val']
        if vals:
            q1, med, q3 = np.percentile(vals, [25, 50, 75])
            display_name = 'S3VO (Ablated)' if algo == 'javoa_ablated' else ('S3VO' if algo == 'javoa' else ('ORCA' if algo == 'rvo' else algo.upper()))
            print(f"{display_name}: Q1={q1:.2f}, Median={med:.2f}, Q3={q3:.2f}")
            
    if all(len(vals) == 0 for vals in data_list):
        plt.close()
        return

    boxprops = dict(linewidth=1.5)
    whiskerprops = dict(linewidth=1.5)
    capprops = dict(linewidth=1.5)
    medianprops = dict(linewidth=2.0, color='black')

    box = plt.boxplot(data_list, labels=labels, patch_artist=True, widths=0.3, showfliers=False, whis=(0, 100),
                      boxprops=boxprops, whiskerprops=whiskerprops, capprops=capprops, medianprops=medianprops)
    
    for patch, color in zip(box['boxes'], colors):
        patch.set_facecolor(color)
        patch.set_edgecolor('black')
        patch.set_linewidth(1.5)
        
    # Scatter with jitter (Show all data points)
    for i, vals in enumerate(data_list):
        x = np.random.normal(i + 1, 0.04, size=len(vals))
        plt.scatter(x, vals, color='red', alpha=0.5, s=40, zorder=3)
        
    plt.title(title, pad=15, fontsize=26)
    plt.ylabel(ylabel, labelpad=10, fontsize=24)
    plt.xticks(fontsize=22)
    plt.yticks(fontsize=20)
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=300)
    plt.close()
    print(f"Generated plot: {output_path}")

def generate_success_barplot(counts, total, title, ylabel, output_path, color_override=None):
    plt.figure(figsize=(9, 7))
    
    algos = sorted(list(counts.keys()), key=lambda x: x.lower())
    labels = ['S3VO (Ablated)' if algo == 'javoa_ablated' else ('S3VO' if algo == 'javoa' else algo.upper()) for algo in algos]
    values = [counts[algo] for algo in algos]
    
    if color_override:
        plot_colors = [color_override] * len(algos)
    else:
        color_palette = ['lightblue', 'lightcoral', 'lightgreen', 'wheat', 'plum', 'lightgray', 'khaki']
        plot_colors = [color_palette[i % len(color_palette)] for i in range(len(algos))]
    
    bars = plt.bar(labels, values, color=plot_colors, edgecolor='black', linewidth=1.5)
    
    plt.title(f"{title} (n={total})", fontsize=26)
    plt.ylabel(ylabel, fontsize=24)
    plt.xticks(fontsize=22)
    plt.yticks(fontsize=20)
    plt.ylim(0, total + max(total * 0.1, 1))
    
    for bar in bars:
        yval = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2.0, yval + (total*0.01), int(yval), va='bottom', ha='center', fontsize=22, fontweight='bold')

    plt.grid(True, axis='y', linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig(output_path, dpi=300)
    plt.close()
    print(f"Generated plot: {output_path}")

if __name__ == "__main__":
    results_dir = os.path.expanduser('~/ros2_ws/src/avoa3d/results/Final_100')
    if os.path.exists(results_dir):
        analyze_results(results_dir)
    else:
        print(f"Results directory not found: {results_dir}")
