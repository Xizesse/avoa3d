import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button
from mpl_toolkits.mplot3d import Axes3D

class UAVReachableSpaceExplorer:
    def __init__(self):
        self.fig = plt.figure(figsize=(14, 10))
        
        # Set default values
        self.v_x_max = 5.0  # m/s
        self.v_y_max = 4.0  # m/s
        self.v_z_max = 2.0  # m/s
        self.T = 2.0  # seconds
        self.current_v = np.array([1.0, 0.5, 0.0])  # m/s
        self.resolution = 30
        
        # Set up the 3D plot
        self.ax = self.fig.add_subplot(111, projection='3d')
        
        # Adjust the main plot position to make room for sliders
        self.fig.subplots_adjust(bottom=0.4)
        
        # Create sliders
        ax_v_x_max = plt.axes([0.25, 0.25, 0.65, 0.03])
        ax_v_y_max = plt.axes([0.25, 0.2, 0.65, 0.03])
        ax_v_z_max = plt.axes([0.25, 0.15, 0.65, 0.03])
        ax_T = plt.axes([0.25, 0.1, 0.65, 0.03])
        
        # Velocity vector sliders
        ax_v_x = plt.axes([0.25, 0.05, 0.65, 0.03])
        ax_v_y = plt.axes([0.25, 0.0, 0.65, 0.03])
        ax_v_z = plt.axes([0.25, -0.05, 0.65, 0.03])
        
        # Create the sliders
        self.slider_v_x_max = Slider(ax_v_x_max, 'V_x_max (m/s)', 0.1, 10.0, valinit=self.v_x_max)
        self.slider_v_y_max = Slider(ax_v_y_max, 'V_y_max (m/s)', 0.1, 10.0, valinit=self.v_y_max)
        self.slider_v_z_max = Slider(ax_v_z_max, 'V_z_max (m/s)', 0.1, 10.0, valinit=self.v_z_max)
        self.slider_T = Slider(ax_T, 'Time T (s)', 0.1, 5.0, valinit=self.T)
        
        self.slider_v_x = Slider(ax_v_x, 'Current V_x (m/s)', -5.0, 5.0, valinit=self.current_v[0])
        self.slider_v_y = Slider(ax_v_y, 'Current V_y (m/s)', -5.0, 5.0, valinit=self.current_v[1])
        self.slider_v_z = Slider(ax_v_z, 'Current V_z (m/s)', -5.0, 5.0, valinit=self.current_v[2])
        
        # Connect callbacks
        self.slider_v_x_max.on_changed(self.update)
        self.slider_v_y_max.on_changed(self.update)
        self.slider_v_z_max.on_changed(self.update)
        self.slider_T.on_changed(self.update)
        self.slider_v_x.on_changed(self.update)
        self.slider_v_y.on_changed(self.update)
        self.slider_v_z.on_changed(self.update)
        
        # Reset button
        reset_ax = plt.axes([0.8, 0.3, 0.1, 0.04])
        self.button = Button(reset_ax, 'Reset')
        self.button.on_clicked(self.reset)
        
        # Initial plot
        self.update(None)
        
    def update(self, val):
        # Get current values from sliders
        self.v_x_max = self.slider_v_x_max.val
        self.v_y_max = self.slider_v_y_max.val
        self.v_z_max = self.slider_v_z_max.val
        self.T = self.slider_T.val
        self.current_v[0] = self.slider_v_x.val
        self.current_v[1] = self.slider_v_y.val
        self.current_v[2] = self.slider_v_z.val
        
        # Clear the current plot
        self.ax.clear()
        
        # Create new plot
        self.plot_reachable_space()
        
        # Redraw the canvas
        self.fig.canvas.draw_idle()
    
    def reset(self, event):
        # Reset sliders to initial values
        self.slider_v_x_max.reset()
        self.slider_v_y_max.reset()
        self.slider_v_z_max.reset()
        self.slider_T.reset()
        self.slider_v_x.reset()
        self.slider_v_y.reset()
        self.slider_v_z.reset()
    
    def plot_reachable_space(self):
        # Generate points for the ellipsoid surface
        u = np.linspace(0, 2 * np.pi, self.resolution)
        v = np.linspace(0, np.pi, self.resolution)
        
        # Create the ellipsoid
        x = self.v_x_max * self.T * np.outer(np.cos(u), np.sin(v))
        y = self.v_y_max * self.T * np.outer(np.sin(u), np.sin(v))
        z = self.v_z_max * self.T * np.outer(np.ones_like(u), np.cos(v))
        
        # Shift the ellipsoid based on current velocity
        current_pos = self.current_v * self.T
        x = x + current_pos[0]
        y = y + current_pos[1]
        z = z + current_pos[2]
        
        # Plot the ellipsoid surface
        surf = self.ax.plot_surface(x, y, z, cmap='coolwarm', alpha=0.7, linewidth=0, antialiased=True)
        
        # Plot the current position
        self.ax.scatter([current_pos[0]], [current_pos[1]], [current_pos[2]], 
                         color='green', s=100, label='Position after T')
        
        # Plot the origin
        self.ax.scatter([0], [0], [0], color='black', s=100, label='Origin')
        
        # Plot a vector from origin to current position
        if np.any(current_pos != 0):
            self.ax.quiver(0, 0, 0, current_pos[0], current_pos[1], current_pos[2], 
                      color='blue', label='Displacement Vector')
        
        # Calculate the maximum bounds for the axes
        max_bound = max(
            self.v_x_max * self.T * 1.5 + abs(current_pos[0]),
            self.v_y_max * self.T * 1.5 + abs(current_pos[1]),
            self.v_z_max * self.T * 1.5 + abs(current_pos[2])
        )
        
        # Set axis bounds to be equal for proper scaling
        self.ax.set_xlim(-max_bound, max_bound)
        self.ax.set_ylim(-max_bound, max_bound)
        self.ax.set_zlim(-max_bound, max_bound)
        
        # Add labels and title
        self.ax.set_xlabel('X Position (m)')
        self.ax.set_ylabel('Y Position (m)')
        self.ax.set_zlabel('Z Position (m)')
        self.ax.set_title(f'UAV Reachable Space in Time T={self.T:.1f}s\n'
                     f'Max Velocities: [{self.v_x_max:.1f}, {self.v_y_max:.1f}, {self.v_z_max:.1f}] m/s\n'
                     f'Current Velocity: [{self.current_v[0]:.1f}, {self.current_v[1]:.1f}, {self.current_v[2]:.1f}] m/s')
        
        # Add a grid for better perception of depth
        self.ax.grid(True)
        
        # Add legend
        self.ax.legend()
        
        # Calculate and display the volume
        volume = (4/3) * np.pi * (self.v_x_max * self.T) * (self.v_y_max * self.T) * (self.v_z_max * self.T)
        self.ax.text2D(0.05, 0.95, f"Volume: {volume:.2f} m³", transform=self.ax.transAxes)
        
        # Show the distance from origin to current position
        distance = np.linalg.norm(current_pos)
        self.ax.text2D(0.05, 0.9, f"Distance from origin: {distance:.2f} m", transform=self.ax.transAxes)

# Run the interactive explorer
if __name__ == "__main__":
    explorer = UAVReachableSpaceExplorer()
    plt.tight_layout()
    plt.show()