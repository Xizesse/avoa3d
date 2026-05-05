#include "avoa3d/holonomic_sample_generator.hpp"
#include <cmath>

namespace avoa3d
{

//! HOLONOMIC SAMPLE GENERATOR

HolonomicSampleGenerator::HolonomicSampleGenerator(const rclcpp::Logger& logger)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing holonomic sample generator with default parameters");
}

HolonomicSampleGenerator::HolonomicSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing holonomic sample generator");
    (void)node; 
}

std::vector<VelocitySample> HolonomicSampleGenerator::generateSamples(
    const geometry_msgs::msg::Twist& current_velocity,
    const geometry_msgs::msg::Twist& desired_velocity)
{
    std::vector<VelocitySample> samples;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    double min_vx = std::max(current_velocity.linear.x - params_.a_x_max * params_.delta_t, -params_.v_x_max);
    double max_vx = std::min(current_velocity.linear.x + params_.a_x_max * params_.delta_t, params_.v_x_max);
    
    double min_vy = std::max(current_velocity.linear.y - params_.a_y_max * params_.delta_t, -params_.v_y_max);
    double max_vy = std::min(current_velocity.linear.y + params_.a_y_max * params_.delta_t, params_.v_y_max);
    
    double min_vz = std::max(current_velocity.linear.z - params_.a_z_max * params_.delta_t, -params_.v_z_max);
    double max_vz = std::min(current_velocity.linear.z + params_.a_z_max * params_.delta_t, params_.v_z_max);

    std::uniform_real_distribution<> dist_vx(min_vx, max_vx);
    std::uniform_real_distribution<> dist_vy(min_vy, max_vy);
    std::uniform_real_distribution<> dist_vz(min_vz, max_vz);
    
    for (int i = 0; i < params_.num_samples; ++i) {
        samples.push_back(VelocitySample(dist_vx(gen), dist_vy(gen), dist_vz(gen)));
    }
    
    // Always include current and desired velocity if within bounds
    auto include_if_valid = [&](double x, double y, double z) {
        if (x >= min_vx && x <= max_vx && y >= min_vy && y <= max_vy && z >= min_vz && z <= max_vz) {
            samples.push_back(VelocitySample(x, y, z));
        }
    };

    include_if_valid(current_velocity.linear.x, current_velocity.linear.y, current_velocity.linear.z);
    include_if_valid(desired_velocity.linear.x, desired_velocity.linear.y, desired_velocity.linear.z);
    
    return samples;
}

geometry_msgs::msg::Twist HolonomicSampleGenerator::translateToTwist(const VelocitySample& sample)
{
    geometry_msgs::msg::Twist twist;
    twist.linear.x = sample.vx;
    twist.linear.y = sample.vy;
    twist.linear.z = sample.vz;
    return twist;
}

void HolonomicSampleGenerator::setParams(
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
    params_.delta_t = delta_t;
    params_.num_samples = num_samples;
    
    RCLCPP_INFO(logger_, "Set holonomic parameters: [v_max: %.2f, %.2f, %.2f] [a_max: %.2f, %.2f, %.2f]", 
               params_.v_x_max, params_.v_y_max, params_.v_z_max, params_.a_x_max, params_.a_y_max, params_.a_z_max);
}

//! HOLONOMIC ELLIPSOIDAL SAMPLE GENERATOR

HolonomicEllipsoidalSampleGenerator::HolonomicEllipsoidalSampleGenerator(const rclcpp::Logger& logger)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing holonomic ellipsoidal sample generator");
}

HolonomicEllipsoidalSampleGenerator::HolonomicEllipsoidalSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing holonomic ellipsoidal sample generator");
    (void)node;
}

bool HolonomicEllipsoidalSampleGenerator::isInsideEllipsoid(double vx, double vy, double vz, 
                                                           double vx_max, double vy_max, double vz_max) const
{
    double normalized_sum = (vx * vx) / (vx_max * vx_max) + 
                           (vy * vy) / (vy_max * vy_max) + 
                           (vz * vz) / (vz_max * vz_max);
    return normalized_sum <= 1.0;
}

VelocitySample HolonomicEllipsoidalSampleGenerator::generateEllipsoidSample(std::mt19937& gen,
                                                                            double vx_max, double vy_max, double vz_max) const
{
    std::uniform_real_distribution<> dist_r(0.0, 1.0);
    std::uniform_real_distribution<> dist_theta(0.0, 2.0 * M_PI);
    std::uniform_real_distribution<> dist_cos_phi(-1.0, 1.0);
    
    double r = std::cbrt(dist_r(gen));
    double theta = dist_theta(gen);
    double cos_phi = dist_cos_phi(gen);
    double sin_phi = std::sqrt(1.0 - cos_phi * cos_phi);
    
    return VelocitySample(r * vx_max * sin_phi * std::cos(theta),
                         r * vy_max * sin_phi * std::sin(theta),
                         r * vz_max * cos_phi);
}

std::vector<VelocitySample> HolonomicEllipsoidalSampleGenerator::generateSamples(
    const geometry_msgs::msg::Twist& current_velocity,
    const geometry_msgs::msg::Twist& desired_velocity)
{
    std::vector<VelocitySample> samples;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    double min_vx = std::max(current_velocity.linear.x - params_.a_x_max * params_.delta_t, -params_.v_x_max);
    double max_vx = std::min(current_velocity.linear.x + params_.a_x_max * params_.delta_t, params_.v_x_max);
    double min_vy = std::max(current_velocity.linear.y - params_.a_y_max * params_.delta_t, -params_.v_y_max);
    double max_vy = std::min(current_velocity.linear.y + params_.a_y_max * params_.delta_t, params_.v_y_max);
    double min_vz = std::max(current_velocity.linear.z - params_.a_z_max * params_.delta_t, -params_.v_z_max);
    double max_vz = std::min(current_velocity.linear.z + params_.a_z_max * params_.delta_t, params_.v_z_max);
    
    std::uniform_real_distribution<> dist_vx(min_vx, max_vx);
    std::uniform_real_distribution<> dist_vy(min_vy, max_vy);
    std::uniform_real_distribution<> dist_vz(min_vz, max_vz);
    
    int attempts = 0, max_attempts = params_.num_samples * 10;
    while (samples.size() < static_cast<size_t>(params_.num_samples) && attempts < max_attempts) {
        double vx = dist_vx(gen), vy = dist_vy(gen), vz = dist_vz(gen);
        if (isInsideEllipsoid(vx, vy, vz, params_.v_x_max, params_.v_y_max, params_.v_z_max)) {
            samples.push_back(VelocitySample(vx, vy, vz));
        }
        attempts++;
    }
    
    auto include_if_valid = [&](double x, double y, double z) {
        if (x >= min_vx && x <= max_vx && y >= min_vy && y <= max_vy && z >= min_vz && z <= max_vz &&
            isInsideEllipsoid(x, y, z, params_.v_x_max, params_.v_y_max, params_.v_z_max)) {
            samples.push_back(VelocitySample(x, y, z));
        }
    };

    include_if_valid(current_velocity.linear.x, current_velocity.linear.y, current_velocity.linear.z);
    include_if_valid(desired_velocity.linear.x, desired_velocity.linear.y, desired_velocity.linear.z);
    
    return samples;
}

geometry_msgs::msg::Twist HolonomicEllipsoidalSampleGenerator::translateToTwist(const VelocitySample& sample)
{
    geometry_msgs::msg::Twist twist;
    twist.linear.x = sample.vx; twist.linear.y = sample.vy; twist.linear.z = sample.vz;
    return twist;
}

void HolonomicEllipsoidalSampleGenerator::setParams(
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
    RCLCPP_INFO(logger_, "Set holonomic ellipsoidal parameters.");
}

} // namespace avoa3d
