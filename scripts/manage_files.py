import os
import shutil

def manage_files():
    # Base directory paths
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    results_dir = os.path.join(base_dir, 'results')
    
    feb23_dir = os.path.join(results_dir, 'Feb23')
    rand_dir = os.path.join(results_dir, 'randomized_saved_100_21Feb')
    final_dir = os.path.join(results_dir, 'Final_100')

    # Create a results folder called Final_100
    os.makedirs(final_dir, exist_ok=True)
    
    # Get all scenario folders from Feb23 directory
    scenario_folders = []
    if os.path.exists(feb23_dir):
        scenario_folders = [f for f in os.listdir(feb23_dir) if os.path.isdir(os.path.join(feb23_dir, f)) and f.startswith('s')]
    
    for scenario in scenario_folders:
        final_scenario_dir = os.path.join(final_dir, scenario)
        
        # It should look like the other results folder : one folder for each scenario
        os.makedirs(final_scenario_dir, exist_ok=True)
        
        # For RVO and ablated javoa, copy the csv files from the Feb23 folder to the Final_100 folder
        feb23_scenario_dir = os.path.join(feb23_dir, scenario)
        
        for file_name in ['rvo.csv', 'javoa_ablated.csv']:
            src = os.path.join(feb23_scenario_dir, file_name)
            dst = os.path.join(final_scenario_dir, file_name)
            if os.path.exists(src):
                shutil.copy2(src, dst)
                
        # For javoa, copy the csv files from the randomized_saved_100_21Feb
        rand_scenario_dir = os.path.join(rand_dir, scenario)
        javoa_src = os.path.join(rand_scenario_dir, 'javoa.csv')
        javoa_dst = os.path.join(final_scenario_dir, 'javoa.csv')
        
        if os.path.exists(javoa_src):
            shutil.copy2(javoa_src, javoa_dst)
            
    print(f"File management complete. Results saved in: {final_dir}")

if __name__ == '__main__':
    manage_files()
