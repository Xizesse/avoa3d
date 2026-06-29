#include "s3vo/diff_drive_sample_generator.hpp"
#include <cmath>

namespace s3vo
{

//! Diff Drive Sample Generator

DiffDriveSampleGenerator::DiffDriveSampleGenerator(const rclcpp::Logger& logger)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing diff drive generator");
}

std::vector<VelocitySample> DiffDriveSampleGenerator::generateSamples(
    const geometry_msgs::msg::Twist& current_velocity,
    const geometry_msgs::msg::Twist& desired_velocity)
{
    std::vector<VelocitySample> samples;

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
        double v = dist_v(random_engine_);
        double w = dist_w(random_engine_);
        
        // VO-Space: Mapped Cartesian velocity
        double vx = v * std::cos(w * params_.delta_t); 
        double vy = v * std::sin(w * params_.delta_t); 
        
        // Create sample with both domains
        samples.push_back(VelocitySample(vx, vy, 0.0,  v, 0.0, 0.0,  0.0, 0.0, w));
    }
    
    // Helper to add a specific (v, w) pair
    auto add_vw = [&](double v, double w) {
        double vx = v * std::cos(w * params_.delta_t);
        double vy = v * std::sin(w * params_.delta_t);
        samples.push_back(VelocitySample(vx, vy, 0.0,  v, 0.0, 0.0,  0.0, 0.0, w));
    };

    // Always include the current velocity for smoothness 
    if (current_v >= min_v && current_v <= max_v && current_w >= min_w && current_w <= max_w) {
        add_vw(current_v, current_w);
    }
    
    // Include desired velocity
    double desired_v = desired_velocity.linear.x;
    double desired_w = desired_velocity.angular.z;
    if (desired_v >= min_v && desired_v <= max_v && desired_w >= min_w && desired_w <= max_w) {
        add_vw(desired_v, desired_w);
    }
    
    return samples;
}

geometry_msgs::msg::Twist DiffDriveSampleGenerator::translateToTwist(const VelocitySample& sample)
{
    // Now we just return the stored twist directly!
    geometry_msgs::msg::Twist twist;
    twist.linear.x = sample.twist.lx;
    twist.linear.y = sample.twist.ly;
    twist.linear.z = sample.twist.lz;
    twist.angular.x = sample.twist.ax;
    twist.angular.y = sample.twist.ay;
    twist.angular.z = sample.twist.az;
    return twist;
}

double DiffDriveSampleGenerator::normalizeAngle(double angle)
{
    while (angle > M_PI) angle -= 2*M_PI;
    while (angle < -M_PI) angle += 2*M_PI;
    return angle;
}

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

} // namespace s3vo
