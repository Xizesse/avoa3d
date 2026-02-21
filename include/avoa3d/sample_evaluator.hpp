// include/avoa3d/sample_evaluator.hpp
#ifndef AVOA3D_SAMPLE_EVALUATOR_HPP
#define AVOA3D_SAMPLE_EVALUATOR_HPP

#include <vector>
#include <rclcpp/rclcpp.hpp>
#include "geometry_msgs/msg/twist.hpp"
//#include "custom_msgs/msg/element_characteristics_array.hpp"
#include "avoa3d/msg/element_characteristics_stamped.hpp"
#include "avoa3d/msg/element_characteristics_array.hpp"
#include "avoa3d/velocity_sample.hpp"

namespace avoa3d {

class SampleEvaluator {
public:
    SampleEvaluator(rclcpp::Logger logger, double vehicle_radius, 
                    double heading_weight, double danger_weight, double abs_weight, double momentum_weight, double time_to_collision_threshold, double radius_threshold);
    
    // Set the desired velocity
    void setDesiredVelocity(const geometry_msgs::msg::Twist& desired_velocity);
    
    // Main evaluation function (same signature as your original)
    void evaluateSamples(
        std::vector<VelocitySample>& samples, 
        const avoa3d::msg::ElementCharacteristicsArray& obstacles,
        const geometry_msgs::msg::Twist& current_velocity
    );
    
    // Find the best sample
    VelocitySample findBestSample(const std::vector<VelocitySample>& samples);
    
private:
    bool checkCollision(
        const VelocitySample& sample,
        const avoa3d::msg::ElementCharacteristicsStamped& obstacle
    );
    
    // Calculate costs for a sample
    void calculateSampleCost(
        VelocitySample& sample, 
        double desired_vx, 
        double desired_vy, 
        double desired_vz, 
        double desired_magnitude
    );
    
    // Store the desired velocity
    geometry_msgs::msg::Twist latest_desired_velocity_{};
    
    // Logger for messages
    rclcpp::Logger logger_;
    
    // Vehicle configuration
    double vehicle_radius_;
    
    //Weights
    double heading_weight_ = 0.0;
    double danger_weight_ = 0.0;
    double abs_weight_ = 0.0;
    double momentum_weight_ = 0.0;

    // Safety cost parameters
    double time_to_collision_threshold_ = 3000.0; // in seconds
    double radius_threshold_ = 0.2; // in meters
};

} // namespace avoa3d

#endif // AVOA3D_SAMPLE_EVALUATOR_HPP