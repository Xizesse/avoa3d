// include/s3vo/sample_evaluator.hpp
#ifndef S3VO_SAMPLE_EVALUATOR_HPP
#define S3VO_SAMPLE_EVALUATOR_HPP

#include <vector>
#include <rclcpp/rclcpp.hpp>
#include "geometry_msgs/msg/twist.hpp"
#include "s3vo/msg/element_characteristics_stamped.hpp"
#include "s3vo/msg/element_characteristics_array.hpp"
#include "s3vo/velocity_sample.hpp"

namespace s3vo {

class SampleEvaluator {
public:
    SampleEvaluator(rclcpp::Logger logger, double vehicle_radius, 
                    double heading_weight, double danger_weight, double abs_weight, 
                    double momentum_weight, double linear_matching_weight, double twist_matching_weight,
                    double time_to_collision_threshold, double radius_threshold);
    
    void setDesiredVelocity(const geometry_msgs::msg::Twist& desired_velocity);
    
    void evaluateSamples(
        std::vector<VelocitySample>& samples, 
        const s3vo::msg::ElementCharacteristicsArray& obstacles,
        const geometry_msgs::msg::Twist& current_velocity
    );
    
    VelocitySample findBestSample(const std::vector<VelocitySample>& samples);
    
private:
    geometry_msgs::msg::Twist latest_desired_velocity_{};
    rclcpp::Logger logger_;
    double vehicle_radius_;
    
    // Weights
    double heading_weight_ = 0.0;
    double danger_weight_ = 0.0;
    double abs_weight_ = 0.0;
    double momentum_weight_ = 0.0;
    double linear_matching_weight_ = 0.0;
    double twist_matching_weight_ = 0.0;

    // Safety cost parameters
    double time_to_collision_threshold_ = 0.0; 
    double radius_threshold_ = 0.0; 
};

} // namespace s3vo

#endif // S3VO_SAMPLE_EVALUATOR_HPP