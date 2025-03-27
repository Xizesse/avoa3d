#include "avoa3d/sample_generator.hpp"

namespace avoa3d
{

HolonomicSampleGenerator::HolonomicSampleGenerator(const rclcpp::Logger& logger)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    //! Hardcode all motion parameters here
   // Maximum accelerations
   a_x_max_ = 3.0;
   a_y_max_ = 3.0;
   a_z_max_ = 3.0;
   
   // Maximum velocities
   v_x_max_ = 1.0;
   v_y_max_ = 1.0;
   v_z_max_ = 0.0;

   // Time step
   delta_t_ = 1.0;
   
   // Sampling parameters
   num_samples_ = 10000;
   
   /* RCLCPP_INFO(this->get_logger(), "Initializing generator w/ hardcoded parameters:");
   RCLCPP_INFO(this->get_logger(), "  a_x_max: %.2f", a_x_max_);
   RCLCPP_INFO(this->get_logger(), "  a_y_max: %.2f", a_y_max_);
   RCLCPP_INFO(this->get_logger(), "  a_z_max: %.2f", a_z_max_);
   RCLCPP_INFO(this->get_logger(), "  v_x_max: %.2f", v_x_max_);
   RCLCPP_INFO(this->get_logger(), "  v_y_max: %.2f", v_y_max_);
   RCLCPP_INFO(this->get_logger(), "  v_z_max: %.2f", v_z_max_);
   RCLCPP_INFO(this->get_logger(), "  delta_t: %.2f", delta_t_);
   RCLCPP_INFO(this->get_logger(), "  num_samples: %d", num_samples_); */
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

//! Diff Drive Sample Generator

DiffDriveSampleGenerator::DiffDriveSampleGenerator(const rclcpp::Logger& logger)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    //! Hardcode all motion parameters here
   
   // Maximum velocities
   v_linear_max_ = 1.0;
   w_angular_max_ = 1.0;

   // Time step
   delta_t_ = 1.0;
   
   // Sampling parameters
   num_samples_ = 10000;
}


std::vector<VelocitySample> DiffDriveSampleGenerator::generateSamples(
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

        //TODO calculate the bounds 
        
        std::uniform_real_distribution<> dist_v(-v_linear_max_, v_linear_max_);
        std::uniform_real_distribution<> dist_vw(-w_angular_max_, w_angular_max_);
        
        // TODO Generate random samples
        for (int i = 0; i < num_samples_; ++i) {
            double v = dist_v(gen);
            double w = dist_vw(gen);
            //TODO convert to vx, vy, vz
            double vx = v * std::cos(w);
            double vy = v * std::sin(w);
            double vz = 0.0;

            samples.push_back(VelocitySample(vx, vy, vz));
        }
        
/*         // TODO Always include the current velocity as a sample,//TODO but chek if min and max values are okay
        double current_v = std::sqrt(current_velocity.linear.x * current_velocity.linear.x + current_velocity.linear.y * current_velocity.linear.y);
        double current_w = std::atan2(current_velocity.linear.y, current_velocity.linear.x);
        if (current_v >= -v_linear_max_ && current_v <= v_linear_max_ &&
            current_w >= -w_angular_max_ && current_w <= w_angular_max_) {
            samples.push_back(VelocitySample(
                current_velocity.linear.x,
                current_velocity.linear.y,
                current_velocity.linear.z
            ));
        }        
        // TODO Always include the desired velocity as a sample 
        double desired_v = std::sqrt(desired_velocity.linear.x * desired_velocity.linear.x + desired_velocity.linear.y * desired_velocity.linear.y);
        double desired_w = std::atan2(desired_velocity.linear.y, desired_velocity.linear.x);
        if (desired_v >= -v_linear_max_ && desired_v <= v_linear_max_ &&
            desired_w >= -w_angular_max_ && desired_w <= w_angular_max_) {
            samples.push_back(VelocitySample(
                desired_velocity.linear.x,
                desired_velocity.linear.y,
                desired_velocity.linear.z
            ));
        }
         */
        return samples;
    }

    geometry_msgs::msg::Twist DiffDriveSampleGenerator::translateToTwist(const VelocitySample& sample)
    {
        geometry_msgs::msg::Twist twist;
        twist.linear.x = sqrt(sample.vx * sample.vx + sample.vy * sample.vy);
        twist.angular.z = std::atan2(sample.vy, sample.vx);
        return twist;

    }

}

// namespace avoa3d