#ifndef AVOA3D_SAMPLE_GENERATOR_HPP
#define AVOA3D_SAMPLE_GENERATOR_HPP

#include <vector>
#include <random>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "avoa3d/velocity_sample.hpp"
#include "avoa3d/motion_params.hpp" 

namespace avoa3d
{

// Base class for sample generators
class SampleGenerator {
public:
    // Inline destructor ensures no linking issue
    virtual ~SampleGenerator() = default;
    
    // Mark everything pure virtual if the base class has no implementation
    virtual std::vector<VelocitySample> generateSamples(
        const geometry_msgs::msg::Twist& current_velocity,
        const geometry_msgs::msg::Twist& desired_velocity) = 0;
    
    virtual geometry_msgs::msg::Twist translateToTwist(const VelocitySample& sample) = 0;
    
    // Common setter for motion parameters
    virtual void setMotionParameters(const MotionParameters& params) = 0;
    
protected:
    // Protected default constructor prevents direct instantiation
    SampleGenerator() = default;
    
    // Common parameters structure
    MotionParameters params_;
};

// Holonomic sample generator implementation
class HolonomicSampleGenerator : public SampleGenerator
{
public:
    HolonomicSampleGenerator(const rclcpp::Logger& logger);

    HolonomicSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node);

    std::vector<VelocitySample> generateSamples(
        const geometry_msgs::msg::Twist& current_velocity,
        const geometry_msgs::msg::Twist& desired_velocity) override;
    
    geometry_msgs::msg::Twist translateToTwist(const VelocitySample& sample) override;
    
    void setMotionParameters(const MotionParameters& params) override;
    
private:
    rclcpp::Logger logger_;
    std::mt19937 random_engine_;
};

// Diff Drive Sample Generator
class DiffDriveSampleGenerator : public SampleGenerator
{
public:
    DiffDriveSampleGenerator(const rclcpp::Logger& logger);

    DiffDriveSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node);
    
    std::vector<VelocitySample> generateSamples(
        const geometry_msgs::msg::Twist& current_velocity,
        const geometry_msgs::msg::Twist& desired_velocity) override;
         
    geometry_msgs::msg::Twist translateToTwist(const VelocitySample& sample) override;
    
    void setMotionParameters(const MotionParameters& params) override;
    
    double normalizeAngle(double angle);

private:
    rclcpp::Logger logger_;
    std::mt19937 random_engine_;
};

} // namespace avoa3d

#endif // AVOA3D_SAMPLE_GENERATOR_HPP