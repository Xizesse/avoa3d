import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Circle
from matplotlib.widgets import Slider, Button
import matplotlib.colors as colors

class CollisionConeCostVisualizer:
    def __init__(self):
        # Setup the figure and axes
        self.fig, self.ax = plt.subplots(figsize=(12, 10))
        self.fig.subplots_adjust(bottom=0.3)  # Make room for sliders
        
        # Initial parameters
        self.agent_pos = np.array([0, 0])  # Agent at origin
        self.obstacle_pos = np.array([5, 0])  # Obstacle 5 units away on x-axis
        self.obstacle_radius = 1.0  # Radius of the obstacle
        self.max_velocity = 2.0  # Maximum velocity magnitude for the plot
        self.cost_threshold_distance = 5.0  # Distance threshold for cost application (meters)
        
        # Cost function parameters
        self.closest_approach_weight = 1.0  # Weight for closest approach distance
        self.axis_proximity_weight = 1.0  # Weight for proximity to the obstacle axis
        
        # Setup the plot
        self.setup_plot()
        
        # Setup UI controls
        self.setup_controls()
        
        # Draw the initial state
        self.update_plot()
    
    def setup_plot(self):
        """Setup the 2D plot."""
        self.ax.set_xlabel('X Velocity')
        self.ax.set_ylabel('Y Velocity')
        self.ax.set_title('Collision Cone Cost Visualization')
        
        # Set equal aspect ratio
        self.ax.set_aspect('equal')
        
        # Grid
        self.ax.grid(True, linestyle='--', alpha=0.7)
        
        # Create a colorbar for the cost
        self.cost_cmap = plt.cm.viridis_r  # Reversed viridis colormap
        self.norm = colors.Normalize(vmin=0, vmax=1)
        
        # Add a reference point for the agent position in velocity space
        self.ax.plot(0, 0, 'ko', markersize=8, label='Agent')
        
        # Add a text annotation for instructions
        self.ax.text(0.02, 0.98, "Cost increases with: \n- Closer approach to obstacle\n- Proximity to central axis", 
                    transform=self.ax.transAxes, fontsize=10, verticalalignment='top', 
                    bbox=dict(boxstyle='round', facecolor='white', alpha=0.8))
    
    def setup_controls(self):
        """Setup sliders and other UI controls."""
        # Obstacle position sliders
        ax_obstacle_x = plt.axes([0.25, 0.15, 0.65, 0.03])
        ax_obstacle_y = plt.axes([0.25, 0.10, 0.65, 0.03])
        
        self.slider_obstacle_x = Slider(ax_obstacle_x, 'Obstacle X', 0.0, 10.0, valinit=self.obstacle_pos[0])
        self.slider_obstacle_y = Slider(ax_obstacle_y, 'Obstacle Y', -5.0, 5.0, valinit=self.obstacle_pos[1])
        
        # Obstacle radius slider
        ax_obstacle_radius = plt.axes([0.25, 0.05, 0.65, 0.03])
        self.slider_obstacle_radius = Slider(ax_obstacle_radius, 'Obstacle Radius', 0.1, 3.0, valinit=self.obstacle_radius)
        
        # Cost function parameter sliders
        ax_closest_approach_weight = plt.axes([0.25, 0.20, 0.65, 0.03])
        ax_axis_proximity_weight = plt.axes([0.25, 0.25, 0.65, 0.03])
        ax_threshold_distance = plt.axes([0.25, 0.30, 0.65, 0.03])
        
        self.slider_closest_approach_weight = Slider(
            ax_closest_approach_weight, 'Closest Approach Weight', 0.0, 5.0, valinit=self.closest_approach_weight
        )
        self.slider_axis_proximity_weight = Slider(
            ax_axis_proximity_weight, 'Axis Proximity Weight', 0.0, 5.0, valinit=self.axis_proximity_weight
        )
        self.slider_threshold_distance = Slider(
            ax_threshold_distance, 'Cost Threshold Distance (m)', 1.0, 10.0, valinit=self.cost_threshold_distance
        )
        
        # Connect update events
        self.slider_obstacle_x.on_changed(self.update_params)
        self.slider_obstacle_y.on_changed(self.update_params)
        self.slider_obstacle_radius.on_changed(self.update_params)
        self.slider_closest_approach_weight.on_changed(self.update_params)
        self.slider_axis_proximity_weight.on_changed(self.update_params)
        self.slider_threshold_distance.on_changed(self.update_params)
    
    def update_params(self, val):
        """Update parameters based on slider values."""
        self.obstacle_pos = np.array([
            self.slider_obstacle_x.val,
            self.slider_obstacle_y.val
        ])
        self.obstacle_radius = self.slider_obstacle_radius.val
        self.closest_approach_weight = self.slider_closest_approach_weight.val
        self.axis_proximity_weight = self.slider_axis_proximity_weight.val
        self.cost_threshold_distance = self.slider_threshold_distance.val
        self.update_plot()
    
    def calculate_cost(self, velocity_x, velocity_y):
        """
        Calculate the cost for a given velocity vector based on:
        1. Closest approach distance to the obstacle
        2. Proximity to the axis connecting agent and obstacle
        
        Returns a normalized cost value between 0 and 1.
        """
        # Vector from agent to obstacle
        to_obstacle = self.obstacle_pos - self.agent_pos
        distance_to_obstacle = np.linalg.norm(to_obstacle)
        
        # Normalize the direction to the obstacle
        obstacle_direction = to_obstacle / distance_to_obstacle if distance_to_obstacle > 0 else np.array([0, 0])
        
        # Velocity vector
        velocity = np.array([velocity_x, velocity_y])
        velocity_magnitude = np.linalg.norm(velocity)
        
        # If velocity is zero, return zero cost
        if velocity_magnitude < 1e-10:
            return 0
        
        # Calculate time to closest approach
        # v ⋅ (o - a) is the projection of the vector to the obstacle onto the velocity
        v_dot_to_obstacle = np.dot(velocity, to_obstacle)
        
        # If dot product is negative, we're moving away from the obstacle
        if v_dot_to_obstacle <= 0:
            return 0
        
        # Calculate the closest approach distance
        # We project the obstacle vector onto the velocity vector
        # and then calculate the perpendicular component
        projection_length = v_dot_to_obstacle / velocity_magnitude
        closest_approach_vector = to_obstacle - (projection_length / velocity_magnitude) * velocity
        closest_approach_distance = np.linalg.norm(closest_approach_vector)
        
        # Check if this velocity would lead to a collision
        would_collide = closest_approach_distance < self.obstacle_radius
        
        # If we're beyond the threshold distance, no cost
        if closest_approach_distance > self.cost_threshold_distance:
            return 0
        
        # Normalize closest approach distance to get a cost factor
        # Closer to the obstacle = higher cost
        closest_approach_cost = 1.0 - (closest_approach_distance / self.cost_threshold_distance)
        closest_approach_cost = max(0, closest_approach_cost)  # Ensure non-negative
        
        # Calculate angle between velocity and direction to obstacle
        velocity_direction = velocity / velocity_magnitude
        angle = np.arccos(np.clip(np.dot(velocity_direction, obstacle_direction), -1, 1))
        
        # Normalize angle (0 to π/2) to get a cost factor for axis proximity
        # Closer to the axis = higher cost
        normalized_angle = angle / (np.pi/2)
        axis_proximity_cost = 1.0 - normalized_angle
        
        # Combine costs with weights
        total_cost = (self.closest_approach_weight * closest_approach_cost + 
                      self.axis_proximity_weight * axis_proximity_cost) / (self.closest_approach_weight + self.axis_proximity_weight)
        
        # If would collide, max out the cost
        if would_collide:
            total_cost = 1.0
            
        return total_cost
    
    def is_inside_collision_cone(self, velocity_x, velocity_y):
        """
        Check if the given velocity is inside the collision cone.
        
        For a circular obstacle, a velocity vector is inside the collision cone
        if the closest approach distance is less than the obstacle radius.
        """
        # Vector from agent to obstacle
        to_obstacle = self.obstacle_pos - self.agent_pos
        distance = np.linalg.norm(to_obstacle)
        
        # Velocity vector
        velocity = np.array([velocity_x, velocity_y])
        velocity_magnitude = np.linalg.norm(velocity)
        
        # If velocity is zero, it's not inside the cone
        if velocity_magnitude < 1e-10:
            return False
        
        # Calculate time to closest approach
        v_dot_to_obstacle = np.dot(velocity, to_obstacle)
        
        # If dot product is negative, we're moving away from the obstacle
        if v_dot_to_obstacle <= 0:
            return False
        
        # Calculate the closest approach distance
        projection_length = v_dot_to_obstacle / velocity_magnitude
        closest_approach_vector = to_obstacle - (projection_length / velocity_magnitude) * velocity
        closest_approach_distance = np.linalg.norm(closest_approach_vector)
        
        # The agent will collide if the closest approach distance is less than the obstacle radius
        return closest_approach_distance < self.obstacle_radius
    
    def update_plot(self):
        """Update the 2D plot with current parameters."""
        self.ax.clear()
        self.setup_plot()
        
        # Calculate the velocity field grid
        x = np.linspace(-self.max_velocity, self.max_velocity, 100)
        y = np.linspace(-self.max_velocity, self.max_velocity, 100)
        X, Y = np.meshgrid(x, y)
        
        # Calculate cost for each velocity
        Z = np.zeros_like(X)
        for i in range(X.shape[0]):
            for j in range(X.shape[1]):
                Z[i, j] = self.calculate_cost(X[i, j], Y[i, j])
        
        # Plot the cost field as a colored contour
        contour = self.ax.pcolormesh(X, Y, Z, cmap=self.cost_cmap, norm=self.norm, alpha=0.8, shading='auto')
        
        # Add colorbar
        if not hasattr(self, 'cbar'):
            self.cbar = self.fig.colorbar(contour, ax=self.ax)
            self.cbar.set_label('Cost')
        else:
            self.cbar.mappable = contour
            
        # Calculate and plot the collision cone boundary
        # The cone is defined by the tangent lines from the agent to the obstacle circle
        to_obstacle = self.obstacle_pos - self.agent_pos
        distance = np.linalg.norm(to_obstacle)
        
        if distance > self.obstacle_radius:  # Only draw cone if agent is outside obstacle
            # Calculate the half-angle of the collision cone
            sin_alpha = self.obstacle_radius / distance
            alpha = np.arcsin(sin_alpha)
            
            # Direction to obstacle
            direction = to_obstacle / distance
            
            # Calculate perpendicular vector in 2D
            perp = np.array([-direction[1], direction[0]])
            
            # Calculate the tangent points on the obstacle
            tangent_point_1 = self.obstacle_pos + self.obstacle_radius * np.array([np.cos(alpha + np.pi/2), np.sin(alpha + np.pi/2)])
            tangent_point_2 = self.obstacle_pos + self.obstacle_radius * np.array([np.cos(-alpha + np.pi/2), np.sin(-alpha + np.pi/2)])
            
            # Calculate the directions from agent to tangent points
            tangent_dir_1 = tangent_point_1 - self.agent_pos
            tangent_dir_1 = tangent_dir_1 / np.linalg.norm(tangent_dir_1)
            
            tangent_dir_2 = tangent_point_2 - self.agent_pos
            tangent_dir_2 = tangent_dir_2 / np.linalg.norm(tangent_dir_2)
            
            # Calculate points for the cone lines
            cone_length = self.max_velocity * 1.5  # Extend beyond plot limits
            cone_line_1 = np.array([0, 0]) + cone_length * tangent_dir_1
            cone_line_2 = np.array([0, 0]) + cone_length * tangent_dir_2
            
            # Plot the cone lines
            self.ax.plot([0, cone_line_1[0]], [0, cone_line_1[1]], 'r--', linewidth=2, label='Collision Cone')
            self.ax.plot([0, cone_line_2[0]], [0, cone_line_2[1]], 'r--', linewidth=2)
            
            # Draw line to the obstacle center
            obstacle_dir = direction * cone_length
            self.ax.plot([0, obstacle_dir[0]], [0, obstacle_dir[1]], 'k--', linewidth=1, label='Obstacle Direction')
            
            # Add threshold distance circle
            threshold_circle = Circle((0, 0), self.cost_threshold_distance, fill=False, linestyle='--', 
                                     color='blue', linewidth=1, label='Cost Threshold')
            self.ax.add_patch(threshold_circle)
            
        # Set the axis limits
        self.ax.set_xlim(-self.max_velocity, self.max_velocity)
        self.ax.set_ylim(-self.max_velocity, self.max_velocity)
        
        # Add legend
        self.ax.legend(loc='lower right')
        
        # Draw
        self.fig.canvas.draw_idle()

# Run the visualizer
if __name__ == "__main__":
    visualizer = CollisionConeCostVisualizer()
    plt.show()