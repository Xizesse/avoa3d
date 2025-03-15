import numpy as np
import matplotlib.pyplot as plt

# Define robot parameters
v_init = 1.0  # Initial velocity (m/s)
a_max = 2.0   # Maximum acceleration (m/s²)
R_max = 1.5   # Maximum turning radius (m)
t = 1.0       # Time horizon (s)

# Compute max speed possible within t
v_max = v_init + a_max * t
v_min = max(0, v_init - a_max * t)  # Prevent negative speeds

# Define angular velocity constraints
omega_max = v_max / R_max  # Maximum angular velocity (rad/s)
omega_min = v_min / R_max if v_min > 0 else 0  # Avoid division by zero

# Sampling angles for motion
theta_range = np.linspace(-np.pi, np.pi, 200)  # Full range of motion

# Generate reachable positions
reachable_x = []
reachable_y = []

# Straight motion (no turn)
for v in np.linspace(v_min, v_max, 50):
    x = v * t
    reachable_x.append(x)
    reachable_y.append(0)

# Curved motion (left and right turns)
for omega in np.linspace(-omega_max, omega_max, 50):  
    if abs(omega) < 1e-3:  # Avoid numerical issues for near-zero turning rates
        continue
    R = v_max / abs(omega)  # Compute turning radius
    for theta in theta_range:
        x = R * np.sin(omega * t)
        y = R * (1 - np.cos(omega * t)) * np.sign(omega)
        reachable_x.append(x)
        reachable_y.append(y)

# Plot results
plt.figure(figsize=(8, 8))
plt.scatter(reachable_x, reachable_y, s=5, alpha=0.5, label="Reachable Area")
plt.axhline(0, color='k', linestyle='--', linewidth=0.5)
plt.axvline(0, color='k', linestyle='--', linewidth=0.5)
plt.xlabel("X Position (m)")
plt.ylabel("Y Position (m)")
plt.title("Reachable Positions in 1 Second")
plt.legend()
plt.axis("equal")
plt.show()
