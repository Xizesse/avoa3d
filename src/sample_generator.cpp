#include "avoa3d/sample_generator.hpp"

namespace avoa3d
{

//!HOLONOMIC SAMPLE GENERATOR

HolonomicSampleGenerator::HolonomicSampleGenerator(const rclcpp::Logger& logger)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
   // Set default values for motion parameters
   a_x_max_ = 3.0;
   a_y_max_ = 3.0;
   a_z_max_ = 3.0;
   
   // Maximum velocities
   v_x_max_ = 1.0;
   v_y_max_ = 1.0;
   v_z_max_ = 1.0;

   delta_t_ = 1.0;
   
   num_samples_ = 10000;

}

HolonomicSampleGenerator::HolonomicSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
   // Set default values first
   a_x_max_ = 3.0;
   a_y_max_ = 3.0;
   a_z_max_ = 3.0;
   
   v_x_max_ = 1.0;
   v_y_max_ = 1.0;
   v_z_max_ = 1.0;

   delta_t_ = 1.0;
   
   num_samples_ = 10000;
   
   // Then load parameters if node is provided
   if (node != nullptr) {
       loadParams(node);
   } else {
       RCLCPP_WARN(logger_, "No node provided for parameter loading, using default parameters");
   }
}

void HolonomicSampleGenerator::loadParams(const rclcpp::Node* node)
{
    RCLCPP_INFO(logger_, "Loading holonomic parameters from node");
    
    // Get parameters with defaults
    a_x_max_ = node->get_parameter_or("a_x_max", a_x_max_);
    a_y_max_ = node->get_parameter_or("a_y_max", a_y_max_);
    a_z_max_ = node->get_parameter_or("a_z_max", a_z_max_);
    
    v_x_max_ = node->get_parameter_or("v_x_max", v_x_max_);
    v_y_max_ = node->get_parameter_or("v_y_max", v_y_max_);
    v_z_max_ = node->get_parameter_or("v_z_max", v_z_max_);
    
    delta_t_ = node->get_parameter_or("delta_t", delta_t_);
    num_samples_ = node->get_parameter_or("num_samples", num_samples_);
    
    RCLCPP_INFO(logger_, "Loaded holonomic parameters:");
    RCLCPP_INFO(logger_, "  a_x_max: %.2f", a_x_max_);
    RCLCPP_INFO(logger_, "  a_y_max: %.2f", a_y_max_);
    RCLCPP_INFO(logger_, "  a_z_max: %.2f", a_z_max_);
    RCLCPP_INFO(logger_, "  v_x_max: %.2f", v_x_max_);
    RCLCPP_INFO(logger_, "  v_y_max: %.2f", v_y_max_);
    RCLCPP_INFO(logger_, "  v_z_max: %.2f", v_z_max_);
    RCLCPP_INFO(logger_, "  delta_t: %.2f", delta_t_);
    RCLCPP_INFO(logger_, "  num_samples: %d", num_samples_);
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

        double min_vx = std::max(current_velocity.linear.x - a_x_max_ * delta_t_, -v_x_max_);
        double max_vx = std::min(current_velocity.linear.x + a_x_max_ * delta_t_, v_x_max_);
        
        double min_vy = std::max(current_velocity.linear.y - a_y_max_ * delta_t_, -v_y_max_);
        double max_vy = std::min(current_velocity.linear.y + a_y_max_ * delta_t_, v_y_max_);
        
        double min_vz = std::max(current_velocity.linear.z - a_z_max_ * delta_t_, -v_z_max_);
        double max_vz = std::min(current_velocity.linear.z + a_z_max_ * delta_t_, v_z_max_);
    
        // Create distributions based on the calculated bounds
        std::uniform_real_distribution<> dist_vx(min_vx, max_vx);
        std::uniform_real_distribution<> dist_vy(min_vy, max_vy);
        std::uniform_real_distribution<> dist_vz(min_vz, max_vz);
        
        // Generate random samples
        for (int i = 0; i < num_samples_; ++i) {
            double vx = dist_vx(gen);
            double vy = dist_vy(gen);
            double vz = dist_vz(gen);
            
            samples.push_back(VelocitySample(vx, vy, vz));
        }
        
        // Always include the current velocity as a sample,//TODO but chek if min and max values are okay
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

//! Diff Drive Sample Generator

DiffDriveSampleGenerator::DiffDriveSampleGenerator(const rclcpp::Logger& logger)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
   // Set default values
   v_linear_max_ = 1.2;
   w_angular_max_ = 1.0;
   delta_t_ = 1.0;
   num_samples_ = 10000;
   
   RCLCPP_INFO(logger_, "Initializing diff drive generator with default parameters");
}

// Constructor with parameter loading from ROS node
DiffDriveSampleGenerator::DiffDriveSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
   // Set default values first
   v_linear_max_ = 1.2;
   w_angular_max_ = 1.0;
   delta_t_ = 1.0;
   num_samples_ = 10000;
   
   // Then load parameters if node is provided
   if (node != nullptr) {
       loadParams(node);
   } else {
       RCLCPP_WARN(logger_, "No node provided for parameter loading, using default parameters");
   }
}

void DiffDriveSampleGenerator::loadParams(const rclcpp::Node* node)
{
    RCLCPP_INFO(logger_, "Loading diff drive parameters from node");
    
    // Get parameters with defaults
    v_linear_max_ = node->get_parameter_or("v_linear_max", v_linear_max_);
    w_angular_max_ = node->get_parameter_or("w_angular_max", w_angular_max_);
    delta_t_ = node->get_parameter_or("delta_t", delta_t_);
    num_samples_ = node->get_parameter_or("num_samples", num_samples_);
    
    RCLCPP_INFO(logger_, "Loaded diff drive parameters:");
    RCLCPP_INFO(logger_, "  v_linear_max: %.2f", v_linear_max_);
    RCLCPP_INFO(logger_, "  w_angular_max: %.2f", w_angular_max_);
    RCLCPP_INFO(logger_, "  delta_t: %.2f", delta_t_);
    RCLCPP_INFO(logger_, "  num_samples: %d", num_samples_);
}

std::vector<VelocitySample> DiffDriveSampleGenerator::generateSamples(
    const geometry_msgs::msg::Twist& current_velocity,
    const geometry_msgs::msg::Twist& desired_velocity)
{
    std::vector<VelocitySample> samples;
    std::random_device rd;
    // random number generation
    std::mt19937 gen(rd());
    
    std::uniform_real_distribution<> dist_v(-v_linear_max_, v_linear_max_);
    std::uniform_real_distribution<> dist_w(-w_angular_max_, w_angular_max_);
    
    // Generate random samples
    for (int i = 0; i < num_samples_; ++i) {
        double v = dist_v(gen);
        double w = dist_w(gen);
        
        // Convert to vx, vy, vz
        double vx = v * std::cos(w);
        double vy = v * std::sin(w);
        double vz = 0.0;
        
        samples.push_back(VelocitySample(vx, vy, vz));
    }
    
    // Could add current and desired velocity here if needed
    
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
    twist.angular.z = std::atan2(sample.vy, sample.vx);
    if (sample.vx <= 0.0) {
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

} // namespace avoa3d