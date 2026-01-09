#!/usr/bin/env python3

import argparse
import csv
import math
import os
from datetime import datetime

import numpy as np

try:
    import plotly.graph_objects as go
    PLOTLY_AVAILABLE = True
except ImportError:
    PLOTLY_AVAILABLE = False

try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from matplotlib.patches import Circle
    PLOTTING_AVAILABLE = True
except ImportError:
    PLOTTING_AVAILABLE = False

# ---------------------------
# Default parameters (edit here)
# ---------------------------

DEFAULT_INPUT_FOLDER = "/home/xizesse/ros2_ws/src/avoa3d/results/c7_RVO"
DEFAULT_CSV_NAME = "raw_odometry.csv"
DEFAULT_AGENT_RADIUS = 0.867
DEFAULT_OBSTACLE_RADIUS = 0.5
DEFAULT_PROTECTIVE_ZONE = 0.5
DEFAULT_GENERATE_PLOTS = True
DEFAULT_TIME_MARK_INTERVAL = 1.0
DEFAULT_XY_RANGE = (5.0, 15.0, -5.0, 5.0)
DEFAULT_XZ_RANGE = (5.0, 15, -6, 4)


def _find_csv(folder, csv_name=None):
    if csv_name:
        candidate = csv_name if os.path.isabs(csv_name) else os.path.join(folder, csv_name)
        if not os.path.isfile(candidate):
            raise FileNotFoundError(f"CSV not found: {candidate}")
        return candidate

    candidate = os.path.join(folder, "raw_odometry.csv")
    if not os.path.isfile(candidate):
        raise FileNotFoundError("No raw_odometry.csv found in folder")
    return candidate


def _read_csv(csv_path):
    with open(csv_path, "r", newline="") as f:
        reader = csv.reader(f)
        rows = list(reader)

    if not rows:
        raise ValueError("CSV is empty")

    # Optional first line: "Scenario: ..."
    if rows[0] and rows[0][0].startswith("Scenario:"):
        rows = rows[1:]

    if len(rows) < 2:
        raise ValueError("CSV missing header or data rows")

    header = rows[0]
    data_rows = rows[1:]

    def to_float(x):
        try:
            return float(x)
        except ValueError:
            return float("nan")

    data = np.array([[to_float(x) for x in row] for row in data_rows], dtype=float)
    return header, data


def _get_col_idx(header, name):
    try:
        return header.index(name)
    except ValueError:
        return None


def _extract_series(header, data, prefix, count):
    series = {}
    for i in range(count):
        x_idx = _get_col_idx(header, f"{prefix}{i}_x")
        y_idx = _get_col_idx(header, f"{prefix}{i}_y")
        z_idx = _get_col_idx(header, f"{prefix}{i}_z")
        if x_idx is None or y_idx is None or z_idx is None:
            continue
        series[i] = (data[:, x_idx], data[:, y_idx], data[:, z_idx])
    return series


def _add_robot_fading_spheres(fig, time_stamps, pos_x, pos_y, pos_z, interval=2.0,
                              radius=0.71, show_labels=True):
    import plotly.graph_objects as go

    key_frames = []
    for i, t in enumerate(time_stamps):
        if not key_frames or t - key_frames[-1][0] >= interval:
            key_frames.append((t, pos_x[i], pos_y[i], pos_z[i]))

    if not key_frames:
        return

    t0, tN = key_frames[0][0], key_frames[-1][0]
    span = max(tN - t0, 1e-6)

    u, v = np.mgrid[0:2 * np.pi:20j, 0:np.pi:10j]
    for t, cx, cy, cz in key_frames:
        if not np.isfinite([cx, cy, cz]).all():
            continue
        alpha = 0.2 + 0.6 * (t - t0) / span
        xs = cx + radius * np.cos(u) * np.sin(v)
        ys = cy + radius * np.sin(u) * np.sin(v)
        zs = cz + radius * np.cos(v)

        fig.add_trace(
            go.Surface(
                x=xs, y=ys, z=zs,
                opacity=alpha,
                showscale=False,
                colorscale=[[0, "blue"], [1, "blue"]],
                hoverinfo="skip",
                name="Agent footprint",
            )
        )
        if show_labels:
            fig.add_trace(
                go.Scatter3d(
                    x=[cx], y=[cy], z=[cz + radius + 0.2],
                    mode="text",
                    text=[f"{round(t - t0):d}s"],
                    textposition="top center",
                    textfont=dict(size=18, color="darkblue"),
                    showlegend=False,
                    hoverinfo="skip",
                )
            )


def _add_obstacle_fading_spheres(fig, obstacle_data, time_stamps, interval=2.0,
                                 radius=0.5, show_labels=True):
    import plotly.graph_objects as go

    u, v = np.mgrid[0:2 * np.pi:10j, 0:np.pi:5j]

    for label, (ox, oy, oz) in obstacle_data.items():
        key_frames = []
        for i, t in enumerate(time_stamps):
            if not np.isfinite([ox[i], oy[i], oz[i]]).all():
                continue
            if not key_frames or t - key_frames[-1][0] >= interval:
                key_frames.append((t, ox[i], oy[i], oz[i]))

        if not key_frames:
            continue

        t0, tN = key_frames[0][0], key_frames[-1][0]
        span = max(tN - t0, 1e-6)

        for t, cx, cy, cz in key_frames:
            alpha = 0.2 + 0.6 * (t - t0) / span
            xs = cx + radius * np.cos(u) * np.sin(v)
            ys = cy + radius * np.sin(u) * np.sin(v)
            zs = cz + radius * np.cos(v)

            fig.add_trace(
                go.Surface(
                    x=xs, y=ys, z=zs,
                    opacity=alpha,
                    showscale=False,
                    colorscale=[[0, "red"], [1, "red"]],
                    hoverinfo="skip",
                    name=f"Obstacle {label} footprint",
                )
            )
            if show_labels:
                fig.add_trace(
                    go.Scatter3d(
                        x=[cx], y=[cy], z=[cz + radius + 0.2],
                        mode="text",
                        text=[f"{round(t - t0):d}s"],
                        textposition="top center",
                        textfont=dict(size=22, color="darkred"),
                        showlegend=False,
                        hoverinfo="skip",
                    )
                )


def compute_stats(header, data, agent_radius, obstacle_radius, protective_zone, output_path):
    if data.size == 0:
        raise ValueError("No data rows")

    t_idx = _get_col_idx(header, "time_elapsed")
    px_idx = _get_col_idx(header, "agent_pos_x")
    py_idx = _get_col_idx(header, "agent_pos_y")
    pz_idx = _get_col_idx(header, "agent_pos_z")

    if None in (t_idx, px_idx, py_idx, pz_idx):
        raise ValueError("Missing agent position columns")

    t = data[:, t_idx]
    px, py, pz = data[:, px_idx], data[:, py_idx], data[:, pz_idx]

    # Path length
    dx = np.diff(px)
    dy = np.diff(py)
    dz = np.diff(pz)
    path_len = np.sum(np.sqrt(dx**2 + dy**2 + dz**2))

    # Clearance per obstacle (center distance - radii + protective zone)
    min_clear = {"main": float("inf")}
    for i in range(10):
        min_clear[i] = float("inf")

    main_idx = _get_col_idx(header, "obstacle_main_x")
    if main_idx is not None:
        main = data[:, main_idx:main_idx + 3]
        for i in range(len(data)):
            if np.isnan(main[i]).any():
                continue
            dist = np.linalg.norm([px[i] - main[i, 0], py[i] - main[i, 1], pz[i] - main[i, 2]])
            clear = dist - agent_radius - obstacle_radius + protective_zone
            min_clear["main"] = min(min_clear["main"], clear)

    obstacles = _extract_series(header, data, "obstacle_", 10)
    for idx, (ox, oy, oz) in obstacles.items():
        for i in range(len(data)):
            if np.isnan([ox[i], oy[i], oz[i]]).any():
                continue
            dist = np.linalg.norm([px[i] - ox[i], py[i] - oy[i], pz[i] - oz[i]])
            clear = dist - agent_radius - obstacle_radius + protective_zone
            min_clear[idx] = min(min_clear[idx], clear)

    with open(output_path, "w") as f:
        f.write("AVOA Performance Statistics – Postprocessed\n")
        f.write("=" * 60 + "\n\n")
        f.write("PATH\n")
        f.write(f"Path Length:  {path_len:.2f} m\n\n")
        f.write("SAFETY (MIN CLEARANCE PER OBSTACLE)\n")
        main_clear = min_clear["main"]
        if main_clear != float("inf"):
            f.write(f"main: {main_clear:.2f} m\n")
            if main_clear < 0:
                f.write("⚠️ Collision detected with main obstacle\n")
        else:
            f.write("main: no data\n")
        for i in range(10):
            clear = min_clear[i]
            if clear != float("inf"):
                f.write(f"obstacle_{i}: {clear:.2f} m\n")
                if clear < 0:
                    f.write(f"⚠️ Collision detected with obstacle_{i}\n")
            else:
                f.write(f"obstacle_{i}: no data\n")
        f.write("\nGenerated: " + datetime.now().strftime("%Y-%m-%d %H:%M:%S") + "\n")


def plot_trajectory(header, data, output_path, show_labels, agent_radius, obstacle_radius,
                    time_interval):
    if not PLOTLY_AVAILABLE:
        raise RuntimeError("plotly is not installed")

    t_idx = _get_col_idx(header, "time_elapsed")
    px_idx = _get_col_idx(header, "agent_pos_x")
    py_idx = _get_col_idx(header, "agent_pos_y")
    pz_idx = _get_col_idx(header, "agent_pos_z")

    if None in (t_idx, px_idx, py_idx, pz_idx):
        raise ValueError("Missing agent position columns for 3D plot")

    t = data[:, t_idx]
    px, py, pz = data[:, px_idx], data[:, py_idx], data[:, pz_idx]
    mask = np.isfinite(px) & np.isfinite(py) & np.isfinite(pz)
    if not np.any(mask):
        raise ValueError("No valid agent positions for 3D plot")

    t = t[mask]
    px, py, pz = px[mask], py[mask], pz[mask]

    pad = 4.0
    x_min, x_max = np.min(px) - pad, np.max(px) + pad
    y_min, y_max = np.min(py) - pad, np.max(py) + pad
    z_min, z_max = np.min(pz) - pad, np.max(pz) + pad

    x_max = max(x_max, 5.0)
    x_min = min(x_min, -5.0)
    y_max = max(y_max, 5.0)
    y_min = min(y_min, -5.0)

    x_range = x_max - x_min
    y_range = y_max - y_min
    z_range = z_max - z_min
    max_range = max(x_range, y_range, z_range)

    aspect_ratio = dict(
        x=x_range / max_range,
        y=y_range / max_range,
        z=z_range / max_range,
    )

    robot_line = go.Scatter3d(
        x=px, y=py, z=pz,
        mode="lines",
        line=dict(color="blue", width=3),
        name="Agent path",
    )

    obstacle_lines = []
    main_idx = _get_col_idx(header, "obstacle_main_x")
    if main_idx is not None:
        ox, oy, oz = data[:, main_idx], data[:, main_idx + 1], data[:, main_idx + 2]
        mask = np.isfinite(ox) & np.isfinite(oy) & np.isfinite(oz)
        if np.any(mask):
            obstacle_lines.append(go.Scatter3d(
                x=ox[mask], y=oy[mask], z=oz[mask],
                mode="lines",
                line=dict(color="red", width=2, dash="dot"),
                name="obstacle_main",
                opacity=0.6,
            ))

    obstacles = _extract_series(header, data, "obstacle_", 10)
    for idx, (ox, oy, oz) in obstacles.items():
        mask = np.isfinite(ox) & np.isfinite(oy) & np.isfinite(oz)
        if not np.any(mask):
            continue
        obstacle_lines.append(go.Scatter3d(
            x=ox[mask], y=oy[mask], z=oz[mask],
            mode="lines",
            line=dict(color="red", width=2, dash="dot"),
            name=f"obstacle_{idx}",
            opacity=0.6,
        ))

    fig = go.Figure(data=[robot_line] + obstacle_lines)

    visual_agent_radius = 0.71
    visual_obstacle_radius = 0.50
    _add_robot_fading_spheres(
        fig, t, px, py, pz, show_labels=show_labels, radius=visual_agent_radius
    )

    obstacle_data_xyz = {}
    if main_idx is not None:
        ox, oy, oz = data[:, main_idx], data[:, main_idx + 1], data[:, main_idx + 2]
        obstacle_data_xyz["main"] = (ox, oy, oz)
    obstacle_data_xyz.update(obstacles)
    _add_obstacle_fading_spheres(
        fig, obstacle_data_xyz, data[:, t_idx], show_labels=show_labels, radius=visual_obstacle_radius
    )

    fig.update_layout(
        scene=dict(
            xaxis=dict(
                title="X [m]",
                title_font=dict(size=20),
                tickfont=dict(size=16),
                tickmode="linear",
                dtick=3.0,
                tickangle=0,
                range=[x_min, x_max],
            ),
            yaxis=dict(
                title="Y [m]",
                title_font=dict(size=20),
                tickfont=dict(size=16),
                tickmode="linear",
                dtick=3.0,
                tickangle=0,
                range=[y_min, y_max],
            ),
            zaxis=dict(
                title="Z [m]",
                title_font=dict(size=20),
                tickfont=dict(size=16),
                tickmode="linear",
                dtick=3.0,
                tickangle=0,
                range=[z_min, z_max],
            ),
            aspectmode="manual",
            aspectratio=aspect_ratio,
        ),
        legend=dict(font=dict(size=16)),
        margin=dict(l=0, r=0, b=0, t=0),
    )

    fig.write_html(
        output_path,
        include_plotlyjs="cdn",
        full_html=True,
        config={"scrollZoom": True, "displaylogo": False},
    )


def plot_xy_trajectory(header, data, output_path, show_labels, agent_radius, obstacle_radius,
                       x_min, x_max, y_min, y_max, time_interval):
    if not PLOTTING_AVAILABLE:
        raise RuntimeError("matplotlib is not installed")

    t_idx = _get_col_idx(header, "time_elapsed")
    px_idx = _get_col_idx(header, "agent_pos_x")
    py_idx = _get_col_idx(header, "agent_pos_y")

    if None in (t_idx, px_idx, py_idx):
        raise ValueError("Missing agent position columns for XY plot")

    t = data[:, t_idx]
    px, py = data[:, px_idx], data[:, py_idx]

    mask = np.isfinite(px) & np.isfinite(py)
    mask &= (px >= x_min) & (px <= x_max) & (py >= y_min) & (py <= y_max)
    if not np.any(mask):
        raise ValueError("No valid XY points in the plotting range")

    t = t[mask]
    px, py = px[mask], py[mask]

    obstacles = _extract_series(header, data, "obstacle_", 10)
    main_idx = _get_col_idx(header, "obstacle_main_x")

    plt.figure(figsize=(10, 10))
    ax = plt.gca()

    circle_interval = time_interval
    last_time = -circle_interval
    t0, tN = t[0], t[-1]
    span = max(tN - t0, 1e-6)
    alpha_min, alpha_max = 0.10, 0.75

    obstacle_colors = [
        "orange", "purple", "brown", "pink", "gray",
        "olive", "cyan", "magenta", "yellow", "lime", "red",
    ]

    for i, ti in enumerate(t):
        if ti - last_time < circle_interval:
            continue
        alpha = alpha_min + alpha_max * (ti - t0) / span

        circ = Circle((px[i], py[i]), agent_radius,
                      edgecolor="blue", facecolor="blue",
                      alpha=alpha, linewidth=1)
        ax.add_patch(circ)
        if show_labels:
            ax.text(px[i], py[i], f"{int(ti)}s",
                    ha="center", va="center", fontsize=16, color="black")

        if main_idx is not None:
            ox = data[:, main_idx][mask][i]
            oy = data[:, main_idx + 1][mask][i]
            if np.isfinite(ox) and np.isfinite(oy):
                circ = Circle((ox, oy), obstacle_radius,
                              edgecolor=obstacle_colors[0], facecolor=obstacle_colors[0],
                              alpha=alpha, linewidth=1)
                ax.add_patch(circ)
                if show_labels:
                    ax.text(ox, oy, f"{int(ti)}s",
                            ha="center", va="center", fontsize=16, color="black")

        for j, (ox, oy, _oz) in obstacles.items():
            ox_i = ox[mask][i]
            oy_i = oy[mask][i]
            if not np.isfinite(ox_i) or not np.isfinite(oy_i):
                continue
            if ox_i < x_min or ox_i > x_max or oy_i < y_min or oy_i > y_max:
                continue
            color = obstacle_colors[(j + 1) % len(obstacle_colors)]
            circ = Circle((ox_i, oy_i), obstacle_radius,
                          edgecolor=color, facecolor=color,
                          alpha=alpha, linewidth=1)
            ax.add_patch(circ)
            if show_labels:
                ax.text(ox_i, oy_i, f"{int(ti)}s",
                        ha="center", va="center", fontsize=16, color="black")

        last_time = ti

    plt.plot(px, py, "b--", linewidth=1, alpha=0.4)
    plt.scatter(px[0], py[0], color="blue", s=120, marker="s",
                alpha=0.8, edgecolors="darkblue", linewidth=2, zorder=5)
    plt.scatter(px[-1], py[-1], color="blue", s=120, marker="s",
                alpha=0.3, edgecolors="darkblue", linewidth=2, zorder=5)

    plt.xlim(x_min, x_max)
    plt.ylim(y_min, y_max)
    plt.grid(True, alpha=0.3)
    plt.xlabel("X Position [m]", fontsize=30, fontweight="bold")
    plt.ylabel("Y Position [m]", fontsize=30, fontweight="bold")
    plt.tick_params(axis="both", which="major", labelsize=30)
    plt.axis("equal")

    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    plt.close()


def plot_xz_trajectory(header, data, output_path, show_labels, agent_radius, obstacle_radius,
                       x_min, x_max, z_min, z_max, time_interval):
    if not PLOTTING_AVAILABLE:
        raise RuntimeError("matplotlib is not installed")

    t_idx = _get_col_idx(header, "time_elapsed")
    px_idx = _get_col_idx(header, "agent_pos_x")
    pz_idx = _get_col_idx(header, "agent_pos_z")

    if None in (t_idx, px_idx, pz_idx):
        raise ValueError("Missing agent position columns for XZ plot")

    t = data[:, t_idx]
    px, pz = data[:, px_idx], data[:, pz_idx]

    mask = np.isfinite(px) & np.isfinite(pz)
    if not np.any(mask):
        raise ValueError("No valid XZ points available")

    t = t[mask]
    px, pz = px[mask], pz[mask]

    if x_min is None or x_max is None:
        pad = 1.0
        x_min = np.min(px) - pad
        x_max = np.max(px) + pad
    if z_min is None or z_max is None:
        pad = 1.0
        z_min = np.min(pz) - pad
        z_max = np.max(pz) + pad

    obstacles = _extract_series(header, data, "obstacle_", 10)
    main_idx = _get_col_idx(header, "obstacle_main_x")

    plt.figure(figsize=(10, 10))
    ax = plt.gca()

    circle_interval = time_interval
    last_time = -circle_interval
    t0, tN = t[0], t[-1]
    span = max(tN - t0, 1e-6)
    alpha_min, alpha_max = 0.10, 0.75

    obstacle_colors = [
        "orange", "purple", "brown", "pink", "gray",
        "olive", "cyan", "magenta", "yellow", "lime", "red",
    ]

    for i, ti in enumerate(t):
        if ti - last_time < circle_interval:
            continue
        alpha = alpha_min + alpha_max * (ti - t0) / span

        circ = Circle((px[i], pz[i]), agent_radius,
                      edgecolor="blue", facecolor="blue",
                      alpha=alpha, linewidth=1)
        ax.add_patch(circ)
        if show_labels:
            ax.text(px[i], pz[i], f"{int(ti)}s",
                    ha="center", va="center", fontsize=16, color="black")

        if main_idx is not None:
            ox = data[:, main_idx][mask][i]
            oz = data[:, main_idx + 2][mask][i]
            if np.isfinite(ox) and np.isfinite(oz):
                circ = Circle((ox, oz), obstacle_radius,
                              edgecolor=obstacle_colors[0], facecolor=obstacle_colors[0],
                              alpha=alpha, linewidth=1)
                ax.add_patch(circ)
                if show_labels:
                    ax.text(ox, oz, f"{int(ti)}s",
                            ha="center", va="center", fontsize=16, color="black")

        for j, (ox, _oy, oz) in obstacles.items():
            ox_i = ox[mask][i]
            oz_i = oz[mask][i]
            if not np.isfinite(ox_i) or not np.isfinite(oz_i):
                continue
            if ox_i < x_min or ox_i > x_max or oz_i < z_min or oz_i > z_max:
                continue
            color = obstacle_colors[(j + 1) % len(obstacle_colors)]
            circ = Circle((ox_i, oz_i), obstacle_radius,
                          edgecolor=color, facecolor=color,
                          alpha=alpha, linewidth=1)
            ax.add_patch(circ)
            if show_labels:
                ax.text(ox_i, oz_i, f"{int(ti)}s",
                        ha="center", va="center", fontsize=16, color="black")

        last_time = ti

    plt.plot(px, pz, "b--", linewidth=1, alpha=0.4)
    plt.scatter(px[0], pz[0], color="blue", s=120, marker="s",
                alpha=0.8, edgecolors="darkblue", linewidth=2, zorder=5)
    plt.scatter(px[-1], pz[-1], color="blue", s=120, marker="s",
                alpha=0.3, edgecolors="darkblue", linewidth=2, zorder=5)

    plt.xlim(x_min, x_max)
    plt.ylim(z_min, z_max)
    plt.grid(True, alpha=0.3)
    plt.xlabel("X Position [m]", fontsize=30, fontweight="bold")
    plt.ylabel("Z Position [m]", fontsize=30, fontweight="bold")
    plt.tick_params(axis="both", which="major", labelsize=30)
    plt.axis("equal")

    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    plt.close()


def plot_velocity_magnitude(header, data, output_path):
    if not PLOTTING_AVAILABLE:
        raise RuntimeError("matplotlib is not installed")

    t_idx = _get_col_idx(header, "time_elapsed")
    vx_idx = _get_col_idx(header, "agent_vel_x")
    vy_idx = _get_col_idx(header, "agent_vel_y")
    vz_idx = _get_col_idx(header, "agent_vel_z")

    if None in (t_idx, vx_idx, vy_idx, vz_idx):
        raise ValueError("Missing agent velocity columns for magnitude plot")

    t = data[:, t_idx]
    vx, vy, vz = data[:, vx_idx], data[:, vy_idx], data[:, vz_idx]

    mask = np.isfinite(t) & np.isfinite(vx) & np.isfinite(vy) & np.isfinite(vz)
    if not np.any(mask):
        raise ValueError("No valid velocity samples for magnitude plot")

    t = t[mask]
    speed = np.sqrt(vx[mask] ** 2 + vy[mask] ** 2 + vz[mask] ** 2)

    plt.figure(figsize=(10, 10))
    plt.plot(t, speed, color="blue", linewidth=2.5, label="speed")
    plt.grid(True, alpha=0.3)
    plt.xlabel("Time [s]", fontsize=26, fontweight="bold")
    plt.ylabel("Speed [m/s]", fontsize=26, fontweight="bold")
    plt.tick_params(axis="both", which="major", labelsize=22)
    plt.legend()
    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    plt.close()


def plot_velocity_components(header, data, output_path):
    if not PLOTTING_AVAILABLE:
        raise RuntimeError("matplotlib is not installed")

    t_idx = _get_col_idx(header, "time_elapsed")
    vx_idx = _get_col_idx(header, "agent_vel_x")
    vy_idx = _get_col_idx(header, "agent_vel_y")
    vz_idx = _get_col_idx(header, "agent_vel_z")

    if None in (t_idx, vx_idx, vy_idx, vz_idx):
        raise ValueError("Missing agent velocity columns for components plot")

    t = data[:, t_idx]
    vx, vy, vz = data[:, vx_idx], data[:, vy_idx], data[:, vz_idx]

    mask = np.isfinite(t) & np.isfinite(vx) & np.isfinite(vy) & np.isfinite(vz)
    if not np.any(mask):
        raise ValueError("No valid velocity samples for components plot")

    t = t[mask]
    vx, vy, vz = vx[mask], vy[mask], vz[mask]

    fig, axs = plt.subplots(3, 1, figsize=(10, 10), sharex=True)
    labels = ["Vx [m/s]", "Vy [m/s]", "Vz [m/s]"]
    colors = ["tab:blue", "tab:orange", "tab:green"]
    series = [vx, vy, vz]

    for i in range(3):
        axs[i].plot(t, series[i], "-", color=colors[i], linewidth=3.0)
        axs[i].set_ylabel(labels[i], fontsize=22, fontweight="bold")
        axs[i].grid(True, alpha=0.3)
        axs[i].tick_params(axis="both", which="major", labelsize=18)

    axs[-1].set_xlabel("Time [s]", fontsize=22, fontweight="bold")
    fig.tight_layout()
    fig.savefig(output_path, dpi=300, bbox_inches="tight")
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser(description="Postprocess raw metrics CSV into plots and stats.")
    parser.add_argument(
        "folder",
        nargs="?",
        default=DEFAULT_INPUT_FOLDER,
        help="Folder containing raw_odometry.csv",
    )
    parser.add_argument("--csv", default=DEFAULT_CSV_NAME, help="CSV filename within the folder")
    parser.add_argument("--agent-radius", type=float, default=DEFAULT_AGENT_RADIUS)
    parser.add_argument("--obstacle-radius", type=float, default=DEFAULT_OBSTACLE_RADIUS)
    parser.add_argument("--protective-zone", type=float, default=DEFAULT_PROTECTIVE_ZONE)
    parser.add_argument("--time-interval", type=float, default=DEFAULT_TIME_MARK_INTERVAL)
    parser.add_argument("--no-plots", action="store_true")
    args = parser.parse_args()

    if not args.folder:
        raise SystemExit("Input folder required (set DEFAULT_INPUT_FOLDER or pass it as an argument).")

    csv_name = args.csv if args.csv else None
    csv_path = _find_csv(args.folder, csv_name)
    header, data = _read_csv(csv_path)

    stats_path = os.path.join(args.folder, "performance_stats.txt")
    compute_stats(
        header,
        data,
        args.agent_radius,
        args.obstacle_radius,
        args.protective_zone,
        stats_path,
    )

    if DEFAULT_GENERATE_PLOTS and not args.no_plots:
        if not PLOTLY_AVAILABLE:
            raise RuntimeError("plotly is not installed (use --no-plots to skip)")
        try:
            plot_trajectory(
                header,
                data,
                os.path.join(args.folder, "trajectory_plot_3d_with_labels.html"),
                True,
                args.agent_radius,
                args.obstacle_radius,
                args.time_interval,
            )
            plot_trajectory(
                header,
                data,
                os.path.join(args.folder, "trajectory_plot_3d_no_labels.html"),
                False,
                args.agent_radius,
                args.obstacle_radius,
                args.time_interval,
            )
        except Exception as exc:
            print(f"Skipping 3D plot generation: {exc}")

        if not PLOTTING_AVAILABLE:
            print("matplotlib is not installed (skipping 2D and velocity plots)")
        else:
            try:
                xy_path = os.path.join(args.folder, "xy_trajectory_plot_with_labels.png")
                plot_xy_trajectory(
                    header,
                    data,
                    xy_path,
                    True,
                    args.agent_radius,
                    args.obstacle_radius,
                    *DEFAULT_XY_RANGE,
                    args.time_interval,
                )
                xy_path = os.path.join(args.folder, "xy_trajectory_plot_no_labels.png")
                plot_xy_trajectory(
                    header,
                    data,
                    xy_path,
                    False,
                    args.agent_radius,
                    args.obstacle_radius,
                    *DEFAULT_XY_RANGE,
                    args.time_interval,
                )
            except Exception as exc:
                print(f"Skipping XY plot generation: {exc}")

            try:
                xz_path = os.path.join(args.folder, "xz_trajectory_plot_with_labels.png")
                plot_xz_trajectory(
                    header,
                    data,
                    xz_path,
                    True,
                    args.agent_radius,
                    args.obstacle_radius,
                    *DEFAULT_XZ_RANGE,
                    args.time_interval,
                )
                xz_path = os.path.join(args.folder, "xz_trajectory_plot_no_labels.png")
                plot_xz_trajectory(
                    header,
                    data,
                    xz_path,
                    False,
                    args.agent_radius,
                    args.obstacle_radius,
                    *DEFAULT_XZ_RANGE,
                    args.time_interval,
                )
            except Exception as exc:
                print(f"Skipping XZ plot generation: {exc}")

            try:
                plot_velocity_magnitude(
                    header,
                    data,
                    os.path.join(args.folder, "velocity_magnitude_plot.png"),
                )
                plot_velocity_components(
                    header,
                    data,
                    os.path.join(args.folder, "velocity_components_plot.png"),
                )
            except Exception as exc:
                print(f"Skipping velocity plot generation: {exc}")

    print(f"Wrote stats: {stats_path}")


if __name__ == "__main__":
    main()
