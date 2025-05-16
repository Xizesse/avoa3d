#include "avoa3d/sample_generator.hpp"

namespace avoa3d
{

//!HOLONOMIC SAMPLE GENERATOR

HolonomicSampleGenerator::HolonomicSampleGenerator(const rclcpp::Logger& logger)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing holonomic sample generator with default parameters");
    // Default parameters already initialized in MotionParameters struct
}

HolonomicSampleGenerator::HolonomicSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing holonomic sample generator");
    // Parameters will be set by the AVOA node
}

void HolonomicSampleGenerator::setMotionParameters(const MotionParameters& params)
{
    params_ = params;
    
    RCLCPP_INFO(logger_, "Set holonomic parameters:");
    RCLCPP_INFO(logger_, "  a_x_max: %.2f", params_.a_x_max);
    RCLCPP_INFO(logger_, "  a_y_max: %.2f", params_.a_y_max);
    RCLCPP_INFO(logger_, "  a_z_max: %.2f", params_.a_z_max);
    RCLCPP_INFO(logger_, "  v_x_max: %.2f", params_.v_x_max);
    RCLCPP_INFO(logger_, "  v_y_max: %.2f", params_.v_y_max);
    RCLCPP_INFO(logger_, "  v_z_max: %.2f", params_.v_z_max);
    RCLCPP_INFO(logger_, "  delta_t: %.2f", params_.delta_t);
    RCLCPP_INFO(logger_, "  num_samples: %d", params_.num_samples);
}

std::vector<VelocitySample> HolonomicSampleGenerator::generateSamples(
    const geometry_msgs::msg::Twist& current_velocity,
    const geometry_msgs::msg::Twist& desired_velocity)
    {
        std::vector<VelocitySample> samples;
        std::random_device rd;
        // random number generation
        std::mt19937 gen(rd());
        
        // Calculate velocity limits based on current velocity and acceleration constraints
        // Lower bounds: current velocity - max acceleration * delta_t (but not below -v_max)
        // Upper bounds: current velocity + max acceleration * delta_t (but not above v_max)

        double min_vx = std::max(current_velocity.linear.x - params_.a_x_max * params_.delta_t, -params_.v_x_max);
        double max_vx = std::min(current_velocity.linear.x + params_.a_x_max * params_.delta_t, params_.v_x_max);
        
        double min_vy = std::max(current_velocity.linear.y - params_.a_y_max * params_.delta_t, -params_.v_y_max);
        double max_vy = std::min(current_velocity.linear.y + params_.a_y_max * params_.delta_t, params_.v_y_max);
        
        double min_vz = std::max(current_velocity.linear.z - params_.a_z_max * params_.delta_t, -params_.v_z_max);
        double max_vz = std::min(current_velocity.linear.z + params_.a_z_max * params_.delta_t, params_.v_z_max);
    
        // Create distributions based on the calculated bounds
        std::uniform_real_distribution<> dist_vx(min_vx, max_vx);
        std::uniform_real_distribution<> dist_vy(min_vy, max_vy);
        std::uniform_real_distribution<> dist_vz(min_vz, max_vz);
        
        // Generate random samples
        for (int i = 0; i < params_.num_samples; ++i) {
            double vx = dist_vx(gen);
            double vy = dist_vy(gen);
            double vz = dist_vz(gen);
            
            samples.push_back(VelocitySample(vx, vy, vz));
        }
        
        // Always include the current velocity as a sample
        if (current_velocity.linear.x >= min_vx && current_velocity.linear.x <= max_vx &&
            current_velocity.linear.y >= min_vy && current_velocity.linear.y <= max_vy &&
            current_velocity.linear.z >= min_vz && current_velocity.linear.z <= max_vz) {
            samples.push_back(VelocitySample(
                current_velocity.linear.x,
                current_velocity.linear.y,
                current_velocity.linear.z
            ));
        }
        
        
        // Always include the desired velocity as a sample 
        if (desired_velocity.linear.x >= min_vx && desired_velocity.linear.x <= max_vx &&
            desired_velocity.linear.y >= min_vy && desired_velocity.linear.y <= max_vy &&
            desired_velocity.linear.z >= min_vz && desired_velocity.linear.z <= max_vz) {
            samples.push_back(VelocitySample(
                desired_velocity.linear.x,
                desired_velocity.linear.y,
                desired_velocity.linear.z
            ));
        }
        
        return samples;
    }

geometry_msgs::msg::Twist HolonomicSampleGenerator::translateToTwist(const VelocitySample& sample)
{
    //return as is
    geometry_msgs::msg::Twist twist;
    twist.linear.x = sample.vx;
    twist.linear.y = sample.vy;
    twist.linear.z = sample.vz;
    twist.angular.x = 0.0;
    twist.angular.y = 0.0;
    twist.angular.z = 0.0;
    return twist;
}

void HolonomicSampleGenerator::setParams(
    double v_x_max, double v_y_max, double v_z_max,
    double a_x_max, double a_y_max, double a_z_max,
    double w_roll_max, double w_pitch_max, double w_yaw_max,
    double a_roll_max, double a_pitch_max, double a_yaw_max,
    double delta_t, int num_samples) 
{
    // Store parameters in the params_ structure
    params_.v_x_max = v_x_max;
    params_.v_y_max = v_y_max;
    params_.v_z_max = v_z_max;
    params_.a_x_max = a_x_max;
    params_.a_y_max = a_y_max;
    params_.a_z_max = a_z_max;
    params_.w_roll_max = w_roll_max;
    params_.w_pitch_max = w_pitch_max;
    params_.w_yaw_max = w_yaw_max;
    params_.a_roll_max = a_roll_max;
    params_.a_pitch_max = a_pitch_max;
    params_.a_yaw_max = a_yaw_max;
    params_.delta_t = delta_t;
    params_.num_samples = num_samples;
    
    // Log parameters
    RCLCPP_INFO(logger_, "Set holonomic parameters:");
    RCLCPP_INFO(logger_, "  Linear velocity limits [X,Y,Z]: [%.2f, %.2f, %.2f]", 
               params_.v_x_max, params_.v_y_max, params_.v_z_max);
    RCLCPP_INFO(logger_, "  Linear accel limits [X,Y,Z]: [%.2f, %.2f, %.2f]", 
               params_.a_x_max, params_.a_y_max, params_.a_z_max);
    RCLCPP_INFO(logger_, "  Angular velocity limits [R,P,Y]: [%.2f, %.2f, %.2f]", 
               params_.w_roll_max, params_.w_pitch_max, params_.w_yaw_max);
    RCLCPP_INFO(logger_, "  delta_t: %.2f, num_samples: %d", params_.delta_t, params_.num_samples);
}

//! Diff Drive Sample Generator

DiffDriveSampleGenerator::DiffDriveSampleGenerator(const rclcpp::Logger& logger)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing diff drive generator with default parameters");
    // Default parameters already initialized in MotionParameters struct
}

DiffDriveSampleGenerator::DiffDriveSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing diff drive generator");
    // Parameters will be set by the AVOA node
}

void DiffDriveSampleGenerator::setMotionParameters(const MotionParameters& params)
{
    params_ = params;
    
    RCLCPP_INFO(logger_, "Set diff drive parameters:");
    RCLCPP_INFO(logger_, "  v_x_max (linear): %.2f", params_.v_x_max);
    RCLCPP_INFO(logger_, "  w_yaw_max (angular): %.2f", params_.w_yaw_max);
    RCLCPP_INFO(logger_, "  delta_t: %.2f", params_.delta_t);
    RCLCPP_INFO(logger_, "  num_samples: %d", params_.num_samples);
}

std::vector<VelocitySample> DiffDriveSampleGenerator::generateSamples(
    const geometry_msgs::msg::Twist& current_velocity,
    const geometry_msgs::msg::Twist& desired_velocity)
{
    std::vector<VelocitySample> samples;
    std::random_device rd;
    // random number generation
    std::mt19937 gen(rd());
    
    std::uniform_real_distribution<> dist_v(-params_.v_x_max, params_.v_x_max);
    std::uniform_real_distribution<> dist_w(-params_.w_yaw_max, params_.w_yaw_max);
    
    // Generate random samples
    for (int i = 0; i < params_.num_samples; ++i) {
        double v = dist_v(gen);
        double w = dist_w(gen);
        
        // Convert to vx, vy, vz
        double vx = v * std::cos(w);
        double vy = v * std::sin(w);
        double vz = 0.0;
        
        samples.push_back(VelocitySample(vx, vy, vz));
    }
    
    return samples;
}

geometry_msgs::msg::Twist DiffDriveSampleGenerator::translateToTwist(const VelocitySample& sample)
{
    if (sample.vx == 0.0 && sample.vy == 0.0) {
        return geometry_msgs::msg::Twist();
    }
    
    geometry_msgs::msg::Twist twist;
    // If x is positive, angle is
    twist.linear.x = sqrt(sample.vx * sample.vx + sample.vy * sample.vy);
    twist.angular.z = 3*std::atan2(sample.vy, sample.vx);
    if (false && sample.vx <= 0.0) {
        twist.linear.x *= -1.0;
        twist.angular.z = normalizeAngle(twist.angular.z + M_PI);
    }
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
    // Store parameters in the params_ structure
    params_.v_x_max = v_x_max;
    params_.v_y_max = v_y_max;
    params_.v_z_max = v_z_max;
    params_.a_x_max = a_x_max;
    params_.a_y_max = a_y_max;
    params_.a_z_max = a_z_max;
    params_.w_roll_max = w_roll_max;
    params_.w_pitch_max = w_pitch_max;
    params_.w_yaw_max = w_yaw_max;
    params_.a_roll_max = a_roll_max;
    params_.a_pitch_max = a_pitch_max;
    params_.a_yaw_max = a_yaw_max;
    params_.delta_t = delta_t;
    params_.num_samples = num_samples;
    
    // Log parameters
    RCLCPP_INFO(logger_, "Set diff drive parameters:");
    RCLCPP_INFO(logger_, "  Linear velocity limits [X,Y,Z]: [%.2f, %.2f, %.2f]", 
               params_.v_x_max, params_.v_y_max, params_.v_z_max);
    RCLCPP_INFO(logger_, "  Linear accel limits [X,Y,Z]: [%.2f, %.2f, %.2f]", 
               params_.a_x_max, params_.a_y_max, params_.a_z_max);
    RCLCPP_INFO(logger_, "  Angular velocity limits [R,P,Y]: [%.2f, %.2f, %.2f]", 
               params_.w_roll_max, params_.w_pitch_max, params_.w_yaw_max);
    RCLCPP_INFO(logger_, "  delta_t: %.2f, num_samples: %d", params_.delta_t, params_.num_samples);
}

} // namespace avoa3d