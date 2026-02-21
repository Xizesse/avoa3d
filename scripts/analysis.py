#!/usr/bin/env python3

import os
import csv
import matplotlib.pyplot as plt
import math
import numpy as np

def analyze_results(base_dir):
    analysis_dir = os.path.join(base_dir, 'analysis')
    os.makedirs(analysis_dir, exist_ok=True)
    
    scenarios_processed = 0
    
    # Data storage
    # { 'rvo': [val1, val2...], 'javoa': [val1, val2...] }
    data = {
        'time_to_goal': {'rvo': [], 'javoa': []},
        'min_clearance': {'rvo': [], 'javoa': []},
        'distance_traveled': {'rvo': [], 'javoa': []}
    }
    
    success_counts = {'rvo': 0, 'javoa': 0}
    
    # Iterate over all subdirectories
    # We assume folders are named s{id}
    # Sort folders to process in order (s000, s001...)
    subdirs = sorted([d for d in os.listdir(base_dir) if d.startswith('s') and d != 'scripts' and os.path.isdir(os.path.join(base_dir, d))])
    
    for dir_name in subdirs:
        scenario_path = os.path.join(base_dir, dir_name)
        rvo_csv = os.path.join(scenario_path, 'rvo.csv')
        javoa_csv = os.path.join(scenario_path, 'javoa.csv')
        
        # Check if both exist
        if not (os.path.exists(rvo_csv) and os.path.exists(javoa_csv)):
            # print(f"Skipping {dir_name}: Missing one or both CSVs.")
            continue
            
        print(f"Processing {dir_name}...")
        
        try:
            # Process RVO
            rvo_time, rvo_clearance, rvo_dist, rvo_goal = process_single_csv(rvo_csv)
            # Process Javoa
            jav_time, jav_clearance, jav_dist, jav_goal = process_single_csv(javoa_csv)
            
            if rvo_time is not None and jav_time is not None:
                scenarios_processed += 1
                
                if rvo_goal:
                    success_counts['rvo'] += 1
                    data['time_to_goal']['rvo'].append(rvo_time)
                    data['min_clearance']['rvo'].append(rvo_clearance)
                    data['distance_traveled']['rvo'].append(rvo_dist)
                    
                if jav_goal:
                    success_counts['javoa'] += 1
                    data['time_to_goal']['javoa'].append(jav_time)
                    data['min_clearance']['javoa'].append(jav_clearance)
                    data['distance_traveled']['javoa'].append(jav_dist)
                
        except Exception as e:
            print(f"Error analyzing {dir_name}: {e}")
            
    if scenarios_processed == 0:
        print("No valid scenarios found with both algorithms data.")
        return

    print(f"\nAnalysis complete. Processed {scenarios_processed} scenarios.")
    
    # Generate Box Plots
    generate_boxplot(
        data['time_to_goal'], 
        'Time to Reach Goal Distribution', 
        'Time (s)', 
        os.path.join(analysis_dir, 'time_to_goal_boxplot.png')
    )
    
    generate_boxplot(
        data['min_clearance'], 
        'Minimum Clearance Distribution', 
        'Clearance (m)', 
        os.path.join(analysis_dir, 'min_clearance_boxplot.png')
    )

    generate_boxplot(
        data['distance_traveled'], 
        'Distance Traveled Distribution', 
        'Distance (m)', 
        os.path.join(analysis_dir, 'distance_traveled_boxplot.png')
    )

    generate_success_barplot(
        success_counts,
        scenarios_processed,
        'Goal Reach Rate (< 5m)',
        'Number of Scenarios',
        os.path.join(analysis_dir, 'goal_reached_barplot.png')
    )

def process_single_csv(csv_file):
    rows = []
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(row)
            
    if not rows:
        return None, None, None, False
        
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
                clr = dist - 1.5
                
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

    return time_to_goal, min_clearance_global, distance_traveled, goal_reached

def generate_boxplot(data_dict, title, ylabel, output_path):
    plt.figure(figsize=(8, 6))
    
    labels = ['Javoa', 'RVO']
    values = [data_dict['javoa'], data_dict['rvo']]
    
    if len(values[0]) == 0 and len(values[1]) == 0:
        print(f"Skipping plot {title} because there is no matching data.")
        plt.close()
        return
    
    # Create boxplot
    # patch_artist=True allowing for color filling
    # whis=(0, 100) extends whiskers to min/max data points (no outliers shown separately)
    
    clean_values = [v if len(v) > 0 else [np.nan] for v in values]
    
    bplot = plt.boxplot(clean_values, labels=labels, patch_artist=True, medianprops=dict(color="black"), whis=(0, 100))
    
    # Colors
    colors = ['lightblue', 'lightcoral']
    for patch, color in zip(bplot['boxes'], colors):
        patch.set_facecolor(color)
        
    plt.title(title)
    plt.ylabel(ylabel)
    plt.grid(True, axis='y', linestyle='--', alpha=0.7)
    
    # Add scattered data points for visibility
    # Add some jitter to x
    for i, data in enumerate(values):
        y = data
        x = np.random.normal(1 + i, 0.04, size=len(y))
        plt.plot(x, y, 'r.', alpha=0.5)

    plt.savefig(output_path)
    plt.close()
    print(f"Generated plot: {output_path}")

def generate_success_barplot(counts, total, title, ylabel, output_path):
    plt.figure(figsize=(8, 6))
    
    labels = ['Javoa', 'RVO']
    values = [counts['javoa'], counts['rvo']]
    
    bars = plt.bar(labels, values, color=['lightblue', 'lightcoral'])
    
    plt.title(f"{title} (Total Scenarios: {total})")
    plt.ylabel(ylabel)
    plt.ylim(0, total + max(total * 0.1, 1))
    
    for bar in bars:
        yval = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2.0, yval, int(yval), va='bottom', ha='center')

    plt.grid(True, axis='y', linestyle='--', alpha=0.7)
    plt.savefig(output_path)
    plt.close()
    print(f"Generated plot: {output_path}")

if __name__ == "__main__":
    results_dir = os.path.expanduser('~/ros2_ws/src/avoa3d/results/randomized')
    if os.path.exists(results_dir):
        analyze_results(results_dir)
    else:
        print(f"Results directory not found: {results_dir}")
