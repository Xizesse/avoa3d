#ifndef AVOA3D_SAMPLE_GENERATOR_HPP
#define AVOA3D_SAMPLE_GENERATOR_HPP

#include <vector>
#include <random>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "avoa3d/velocity_sample.hpp"

namespace avoa3d
{

// !Base class for sample generators
class SampleGenerator
{
public:
    virtual ~SampleGenerator() = default;
    
    virtual std::vector<VelocitySample> generateSamples(
        const geometry_msgs::msg::Twist& current_velocity,
        const geometry_msgs::msg::Twist& desired_velocity) = 0;
};

// !Holonomic sample generator implementation
class HolonomicSampleGenerator : public SampleGenerator
{
public:
    HolonomicSampleGenerator(const rclcpp::Logger& logger);
    
    std::vector<VelocitySample> generateSamples(
        const geometry_msgs::msg::Twist& current_velocity,
        const geometry_msgs::msg::Twist& desired_velocity) override;
    
        

private:
    rclcpp::Logger logger_;
    std::mt19937 random_engine_;
    
    // Motion parameters
    double a_x_max_;
    double a_y_max_;
    double a_z_max_;
    double v_x_max_;
    double v_y_max_;
    double v_z_max_;
    double delta_t_;
    int num_samples_;
};



} // namespace avoa3d

#endif // AVOA3D_SAMPLE_GENERATOR_HPP