import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.animation as animation
from matplotlib.widgets import Slider, Button, RadioButtons

class CollisionConeValidator:
    def __init__(self):
        # Setup the figure and axes
        self.fig = plt.figure(figsize=(14, 10))
        self.ax = self.fig.add_subplot(111, projection='3d')
        self.fig.subplots_adjust(bottom=0.25)  # Make room for sliders
        
        # Initial parameters
        self.agent_pos = np.array([0, 0, 0])  # Agent at origin
        self.obstacle_pos = np.array([5, 0, 0])  # Obstacle 5 units away on x-axis
        self.obstacle_radius = 1.0  # Radius of the obstacle
        self.velocity_magnitude = 1.0  # Magnitude of agent's velocity
        self.velocity_direction = np.array([1, 0, 0])  # Initial velocity direction (normalized)
        
        # Setup the plot
        self.setup_plot()
        
        # Setup UI controls
        self.setup_controls()
        
        # Draw the initial state
        self.update_plot()
    
    def setup_plot(self):
        """Setup the 3D plot."""
        self.ax.set_xlabel('X')
        self.ax.set_ylabel('Y')
        self.ax.set_zlabel('Z')
        self.ax.set_title('Collision Cone Visualization')
        
        # Set the axis limits
        self.ax.set_xlim(-10, 10)
        self.ax.set_ylim(-10, 10)
        self.ax.set_zlim(-10, 10)
        
        # Equal aspect ratio
        self.ax.set_box_aspect([1, 1, 1])
        
        # Add a text annotation for collision status
        self.collision_text = self.ax.text2D(0.05, 0.95, "", transform=self.ax.transAxes)
    
    def setup_controls(self):
        """Setup sliders and other UI controls."""
        # Obstacle position sliders
        ax_obstacle_x = plt.axes([0.25, 0.15, 0.65, 0.03])
        ax_obstacle_y = plt.axes([0.25, 0.10, 0.65, 0.03])
        ax_obstacle_z = plt.axes([0.25, 0.05, 0.65, 0.03])
        
        self.slider_obstacle_x = Slider(ax_obstacle_x, 'Obstacle X', -10.0, 10.0, valinit=self.obstacle_pos[0])
        self.slider_obstacle_y = Slider(ax_obstacle_y, 'Obstacle Y', -10.0, 10.0, valinit=self.obstacle_pos[1])
        self.slider_obstacle_z = Slider(ax_obstacle_z, 'Obstacle Z', -10.0, 10.0, valinit=self.obstacle_pos[2])
        
        # Obstacle radius slider
        ax_obstacle_radius = plt.axes([0.25, 0.20, 0.65, 0.03])
        self.slider_obstacle_radius = Slider(ax_obstacle_radius, 'Obstacle Radius', 0.1, 3.0, valinit=self.obstacle_radius)
        
        # Connect update events
        self.slider_obstacle_x.on_changed(self.update_params)
        self.slider_obstacle_y.on_changed(self.update_params)
        self.slider_obstacle_z.on_changed(self.update_params)
        self.slider_obstacle_radius.on_changed(self.update_params)
        
        # Velocity sliders
        ax_velocity_theta = plt.axes([0.1, 0.25, 0.8, 0.03])
        ax_velocity_phi = plt.axes([0.1, 0.30, 0.8, 0.03])
        ax_velocity_magnitude = plt.axes([0.1, 0.35, 0.8, 0.03])
        
        self.slider_velocity_theta = Slider(ax_velocity_theta, 'Velocity θ (degrees)', 0, 360, valinit=0)
        self.slider_velocity_phi = Slider(ax_velocity_phi, 'Velocity φ (degrees)', 0, 180, valinit=90)
        self.slider_velocity_magnitude = Slider(ax_velocity_magnitude, 'Velocity Magnitude', 0.1, 5.0, valinit=self.velocity_magnitude)
        
        self.slider_velocity_theta.on_changed(self.update_velocity)
        self.slider_velocity_phi.on_changed(self.update_velocity)
        self.slider_velocity_magnitude.on_changed(self.update_velocity)
    
    def update_params(self, val):
        """Update obstacle parameters based on slider values."""
        self.obstacle_pos = np.array([
            self.slider_obstacle_x.val,
            self.slider_obstacle_y.val,
            self.slider_obstacle_z.val
        ])
        self.obstacle_radius = self.slider_obstacle_radius.val
        self.update_plot()
    
    def update_velocity(self, val):
        """Update velocity based on slider values."""
        # Convert spherical coordinates to cartesian
        theta = np.radians(self.slider_velocity_theta.val)
        phi = np.radians(self.slider_velocity_phi.val)
        r = self.slider_velocity_magnitude.val
        
        x = r * np.sin(phi) * np.cos(theta)
        y = r * np.sin(phi) * np.sin(theta)
        z = r * np.cos(phi)
        
        self.velocity_magnitude = r
        self.velocity_direction = np.array([x, y, z]) / r if r > 0 else np.array([0, 0, 0])
        
        self.update_plot()
    
    def is_inside_collision_cone(self):
        """
        Check if the agent's velocity is inside the collision cone.
        
        For a spherical obstacle, a velocity vector is inside the collision cone
        if the closest approach distance is less than the obstacle radius.
        """
        # Vector from agent to obstacle
        to_obstacle = self.obstacle_pos - self.agent_pos
        
        # Distance to the obstacle
        distance = np.linalg.norm(to_obstacle)
        
        # Direction to the obstacle
        obstacle_direction = to_obstacle / distance if distance > 0 else np.array([0, 0, 0])
        
        # Velocity vector
        velocity = self.velocity_direction * self.velocity_magnitude
        
        # Calculate time to closest approach
        # v ⋅ (o - a) is the projection of the vector to the obstacle onto the velocity
        v_dot_to_obstacle = np.dot(velocity, to_obstacle)
        
        # If dot product is negative, we're moving away from the obstacle
        if v_dot_to_obstacle <= 0:
            return False
        
        # Calculate the closest approach distance
        # We project the obstacle vector onto the velocity vector
        # and then calculate the perpendicular component
        closest_approach_vector = to_obstacle - (v_dot_to_obstacle / (np.linalg.norm(velocity) ** 2)) * velocity
        closest_approach_distance = np.linalg.norm(closest_approach_vector)
        
        # The agent will collide if the closest approach distance is less than the obstacle radius
        return closest_approach_distance < self.obstacle_radius
    
    def update_plot(self):
        """Update the 3D plot with current parameters."""
        self.ax.clear()
        self.setup_plot()
        
        # Draw the agent (at origin)
        self.ax.scatter([self.agent_pos[0]], [self.agent_pos[1]], [self.agent_pos[2]], 
                        color='blue', s=100, label='Agent')
        
        # Draw the obstacle as a sphere
        u = np.linspace(0, 2 * np.pi, 30)
        v = np.linspace(0, np.pi, 30)
        x = self.obstacle_pos[0] + self.obstacle_radius * np.outer(np.cos(u), np.sin(v))
        y = self.obstacle_pos[1] + self.obstacle_radius * np.outer(np.sin(u), np.sin(v))
        z = self.obstacle_pos[2] + self.obstacle_radius * np.outer(np.ones_like(u), np.cos(v))
        self.ax.plot_surface(x, y, z, color='red', alpha=0.5)
        
        # Draw the agent's velocity vector
        velocity = self.velocity_direction * self.velocity_magnitude * 2  # Scale for visibility
        self.ax.quiver(self.agent_pos[0], self.agent_pos[1], self.agent_pos[2],
                      velocity[0], velocity[1], velocity[2],
                      color='green', label='Velocity')
        
        # Visualize the collision cone
        # The cone is defined by the tangent lines from the agent to the obstacle sphere
        to_obstacle = self.obstacle_pos - self.agent_pos
        distance = np.linalg.norm(to_obstacle)
        
        if distance > self.obstacle_radius:  # Only draw cone if agent is outside obstacle
            # Calculate the half-angle of the collision cone
            sin_alpha = self.obstacle_radius / distance
            alpha = np.arcsin(sin_alpha)
            
            # Direction to obstacle
            direction = to_obstacle / distance
            
            # Create an orthonormal basis with the direction to obstacle as one axis
            if np.abs(direction[0]) < 0.9:
                ortho1 = np.array([1.0, 0, 0])
            else:
                ortho1 = np.array([0, 1.0, 0])
                
            ortho1 = ortho1 - direction * np.dot(direction, ortho1)
            ortho1 = ortho1 / np.linalg.norm(ortho1)
            ortho2 = np.cross(direction, ortho1)
            
            # Generate points on the cone's base circle
            theta = np.linspace(0, 2 * np.pi, 50)
            cone_points = np.zeros((len(theta), 3))
            
            for i, t in enumerate(theta):
                # Point on the circle at the obstacle's distance
                cone_point = direction * distance + \
                             self.obstacle_radius * (np.cos(t) * ortho1 + np.sin(t) * ortho2)
                cone_points[i] = cone_point
            
            # Draw lines from agent to points on the cone's base
            for point in cone_points:
                self.ax.plot([self.agent_pos[0], self.agent_pos[0] + point[0]],
                            [self.agent_pos[1], self.agent_pos[1] + point[1]],
                            [self.agent_pos[2], self.agent_pos[2] + point[2]],
                            color='gray', alpha=0.3)
            
            # Draw the cone's base circle
            circle_x = self.agent_pos[0] + cone_points[:, 0]
            circle_y = self.agent_pos[1] + cone_points[:, 1]
            circle_z = self.agent_pos[2] + cone_points[:, 2]
            self.ax.plot(circle_x, circle_y, circle_z, color='orange')
        
        # Check if velocity is inside collision cone
        is_collision = self.is_inside_collision_cone()
        
        # Update collision status text
        self.collision_text.set_text(f"Collision Predicted: {'Yes' if is_collision else 'No'}")
        self.collision_text.set_color('red' if is_collision else 'green')
        
        # Compute and display additional information
        to_obstacle = self.obstacle_pos - self.agent_pos
        distance = np.linalg.norm(to_obstacle)
        
        # Angle between velocity and direction to obstacle
        to_obstacle_norm = to_obstacle / distance if distance > 0 else np.array([0, 0, 0])
        angle = np.arccos(np.clip(np.dot(self.velocity_direction, to_obstacle_norm), -1, 1))
        angle_degrees = np.degrees(angle)
        
        # Display collision cone angle
        if distance > self.obstacle_radius:
            sin_alpha = self.obstacle_radius / distance
            alpha = np.arcsin(sin_alpha)
            alpha_degrees = np.degrees(alpha)
            self.ax.text2D(0.05, 0.90, f"Collision Cone Half-Angle: {alpha_degrees:.2f}°", transform=self.ax.transAxes)
            self.ax.text2D(0.05, 0.85, f"Velocity-Obstacle Angle: {angle_degrees:.2f}°", transform=self.ax.transAxes)
            self.ax.text2D(0.05, 0.80, f"Distance to Obstacle: {distance:.2f}", transform=self.ax.transAxes)
        
        # Set the axis limits based on the scene content
        max_range = max(10, distance + self.obstacle_radius + 2)
        self.ax.set_xlim(-max_range, max_range)
        self.ax.set_ylim(-max_range, max_range)
        self.ax.set_zlim(-max_range, max_range)
        
        # Add legend
        self.ax.legend()
        
        # Draw
        self.fig.canvas.draw_idle()

# Run the validator
if __name__ == "__main__":
    validator = CollisionConeValidator()
    plt.show()