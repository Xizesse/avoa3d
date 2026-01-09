import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# --- Parameters ---
a, b, c = 15, 15, 3   # Ellipsoid radii

# Ellipsoid surface
u = np.linspace(0, 2*np.pi, 80)
v = np.linspace(0, np.pi, 40)
x = a * np.outer(np.cos(u), np.sin(v))
y = b * np.outer(np.sin(u), np.sin(v))
z = c * np.outer(np.ones_like(u), np.cos(v))

fig = plt.figure(figsize=(7,6))
ax = fig.add_subplot(111, projection='3d')

# --- Outer Ellipsoid ---
ax.plot_wireframe(x, y, z, color='black', linewidth=0.5, alpha=0.8)

# --- Outer Sphere (radius 15) ---
r_big = 15
xs_big = r_big * np.outer(np.cos(u), np.sin(v))
ys_big = r_big * np.outer(np.sin(u), np.sin(v))
zs_big = r_big * np.outer(np.ones_like(u), np.cos(v))
ax.plot_wireframe(xs_big, ys_big, zs_big, color='gray', linewidth=0.6, alpha=0.5, linestyle='--')

# --- Inner Sphere (radius 3) ---
r_small = 3
xs_small = r_small * np.outer(np.cos(u), np.sin(v))
ys_small = r_small * np.outer(np.sin(u), np.sin(v))
zs_small = r_small * np.outer(np.ones_like(u), np.cos(v))
ax.plot_wireframe(xs_small, ys_small, zs_small, color='gray', linewidth=0.6, alpha=0.5, linestyle='--')

# --- Base projection plane (z = 0) ---
xx, yy = np.meshgrid(np.linspace(-15, 15, 2), np.linspace(-15, 15, 2))
zz = np.zeros_like(xx)
ax.plot_surface(xx, yy, zz, color='white', edgecolor='black', linewidth=0.3, alpha=0.05)

# --- Fixed ranges and true cubic scaling ---
ax.set_xlim(-15, 15)
ax.set_ylim(-15, 15)
ax.set_zlim(-15, 15)
ax.set_box_aspect([1, 1, 1])   # 🔥 <- This enforces 1:1:1 visual scale

# --- View and style ---
ax.view_init(elev=25, azim=-45)
ax.set_xlabel(r'$x_b$', labelpad=10)
ax.set_ylabel(r'$y_b$', labelpad=10)
ax.set_zlabel(r'$z_b$', labelpad=10)

# Clean paper look
for axis in [ax.xaxis, ax.yaxis, ax.zaxis]:
    axis.pane.fill = False
ax.grid(False)

plt.tight_layout()
plt.show()
