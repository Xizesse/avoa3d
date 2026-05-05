#include "avoa3d/diff_drive_sample_generator.hpp"
#include <cmath>

namespace avoa3d
{

//! Diff Drive Sample Generator

DiffDriveSampleGenerator::DiffDriveSampleGenerator(const rclcpp::Logger& logger)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing diff drive generator");
}

DiffDriveSampleGenerator::DiffDriveSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing diff drive generator");
    (void)node;
}

/**
 * @brief Generates velocity samples for a differential drive vehicle.
 * 
 * This method implements a Dynamic Window approach:
 * 1. It samples Linear Velocity (v) and Angular Velocity (w) within the reachable 
 *    window defined by acceleration limits and time step (delta_t).
 * 2. It then transposes these (v, w) samples into Cartesian (vx, vy) points.
 *    This allows the SampleEvaluator to perform standard Velocity Obstacle (VO) 
 *    math in a linear velocity space.
 * 
 * @param current_velocity The robot's current state (expects linear.x as v, angular.z as w). 
 *        Used here for reachability analysis.
 * @param desired_velocity The goal velocity (expects linear.x/y as Cartesian goal). 
 *        Used to allways check the desired vel
 * @return std::vector<VelocitySample> A cloud of reachable Cartesian velocity vectors.
 */
std::vector<VelocitySample> DiffDriveSampleGenerator::generateSamples(
    const geometry_msgs::msg::Twist& current_velocity,
    const geometry_msgs::msg::Twist& desired_velocity)
{
    std::vector<VelocitySample> samples;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Extract current linear and angular velocities (v, w space)
    double current_v = current_velocity.linear.x; 
    double current_w = current_velocity.angular.z;
    
    // Calculate reachable velocity bounds based on acceleration limits
    double min_v = std::max(current_v - params_.a_x_max * params_.delta_t, 0.0); // No negative linear velocity
    double max_v = std::min(current_v + params_.a_x_max * params_.delta_t, params_.v_x_max);
    
    double min_w = std::max(current_w - params_.a_yaw_max * params_.delta_t, -params_.w_yaw_max);
    double max_w = std::min(current_w + params_.a_yaw_max * params_.delta_t, params_.w_yaw_max);
        
    std::uniform_real_distribution<> dist_v(min_v, max_v);
    std::uniform_real_distribution<> dist_w(min_w, max_w);
    
    // Generate random (v, w) pairs and transpose to (vx, vy) Cartesian space
    for (int i = 0; i < params_.num_samples; ++i) {
        double v = dist_v(gen);
        double w = dist_w(gen);
        
        // The transposition: vx and vy represent the position the robot would 
        // reach if it followed the arc defined by (v, w) for delta_t.
        double vx = v * std::cos(w * params_.delta_t); 
        double vy = v * std::sin(w * params_.delta_t); 
        samples.push_back(VelocitySample(vx, vy, 0.0));
    }
    
    // Always include the current velocity for smoothness 
    if (current_v >= min_v && current_v <= max_v && current_w >= min_w && current_w <= max_w) {
        double vx_current = current_v * std::cos(current_w * params_.delta_t);
        double vy_current = current_v * std::sin(current_w * params_.delta_t);
        samples.push_back(VelocitySample(vx_current, vy_current, 0.0));
    }
    
    // MAP Desired from vw to xy
    double desired_v = desired_velocity.linear.x;
    double desired_w = desired_velocity.angular.z;
    
    double vx_desired = desired_v * std::cos(desired_w * params_.delta_t);
    double vy_desired = desired_v * std::sin(desired_w * params_.delta_t);

    if (desired_v >= min_v && desired_v <= max_v && desired_w >= min_w && desired_w <= max_w) {
        samples.push_back(VelocitySample(vx_desired, vy_desired, 0.0));
    }
    
    return samples;
}

/**
 * @brief Translates a chosen Cartesian velocity sample back into robot commands (v, w).
 * 
 * After the Evaluator picks the safest/best (vx, vy) vector, we need to convert it 
 * back into the Differential Drive commands that the USV hardware understands.
 * 
 * @param sample The chosen Cartesian velocity vector.
 * @return geometry_msgs::msg::Twist A twist message with linear.x as speed and angular.z as turn rate.
 */
geometry_msgs::msg::Twist DiffDriveSampleGenerator::translateToTwist(const VelocitySample& sample)
{
    geometry_msgs::msg::Twist twist;
    if (std::abs(sample.vx) == 0.0 && std::abs(sample.vy) == 0.0) return twist;
    
    // Recover the angle (heading change) from the Cartesian vector
    double angle = std::atan2(sample.vy, sample.vx);
    
    // Set linear speed (magnitude of the vector)
    twist.linear.x = std::sqrt(sample.vx * sample.vx + sample.vy * sample.vy);
    
    // Set angular speed (angle over time)
    twist.angular.z = angle / params_.delta_t;
    
    return twist;
}

double DiffDriveSampleGenerator::normalizeAngle(double angle)
{
    while (angle > M_PI) angle -= 2*M_PI;
    while (angle < -M_PI) angle += 2*M_PI;
    return angle;
}

/**
 * @brief Updates the kinematic and dynamic constraints for the generator.
 */
void DiffDriveSampleGenerator::setParams(
    double v_x_max, double v_y_max, double v_z_max,
    double a_x_max, double a_y_max, double a_z_max,
    double w_roll_max, double w_pitch_max, double w_yaw_max,
    double a_roll_max, double a_pitch_max, double a_yaw_max,
    double delta_t, int num_samples) 
{
    params_.v_x_max = v_x_max; params_.v_y_max = v_y_max; params_.v_z_max = v_z_max;
    params_.a_x_max = a_x_max; params_.a_y_max = a_y_max; params_.a_z_max = a_z_max;
    params_.w_roll_max = w_roll_max; params_.w_pitch_max = w_pitch_max; params_.w_yaw_max = w_yaw_max;
    params_.a_roll_max = a_roll_max; params_.a_pitch_max = a_pitch_max; params_.a_yaw_max = a_yaw_max;
    params_.delta_t = delta_t; params_.num_samples = num_samples;
    
    RCLCPP_INFO(logger_, "Set diff drive parameters: [v_max: %.2f] [w_max: %.2f] [a_v_max: %.2f] [a_w_max: %.2f]", 
               params_.v_x_max, params_.w_yaw_max, params_.a_x_max, params_.a_yaw_max);
}

} // namespace avoa3d
