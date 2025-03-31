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
class SampleGenerator {
    public:
        // Inline destructor ensures no linking issue
        virtual ~SampleGenerator() = default;
    
        // Mark everything pure virtual if the base class has no implementation
        virtual std::vector<VelocitySample> generateSamples(
          const geometry_msgs::msg::Twist& current_velocity,
          const geometry_msgs::msg::Twist& desired_velocity) = 0;
    
        virtual geometry_msgs::msg::Twist translateToTwist(const VelocitySample& sample) = 0;
    
    protected:
        // Protected default constructor prevents direct instantiation
        SampleGenerator() = default;
    };

// !Holonomic sample generator implementation
class HolonomicSampleGenerator : public SampleGenerator
{
public:
    HolonomicSampleGenerator(const rclcpp::Logger& logger);

    HolonomicSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node);

    std::vector<VelocitySample> generateSamples(
        const geometry_msgs::msg::Twist& current_velocity,
        const geometry_msgs::msg::Twist& desired_velocity) override;
    
    geometry_msgs::msg::Twist translateToTwist(const VelocitySample& sample); //Does noth
    

private:
    rclcpp::Logger logger_;
    std::mt19937 random_engine_;
    
    void loadParams(const rclcpp::Node* node);

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

// !Diff Drive Sample Generator
class DiffDriveSampleGenerator : public SampleGenerator
{
public:
    DiffDriveSampleGenerator(const rclcpp::Logger& logger);

    DiffDriveSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node);
    
    std::vector<VelocitySample> generateSamples(
        const geometry_msgs::msg::Twist& current_velocity,
        const geometry_msgs::msg::Twist& desired_velocity) override;
         
    geometry_msgs::msg::Twist translateToTwist(const VelocitySample& sample) override;
    double normalizeAngle(double angle);

private:
    rclcpp::Logger logger_;
    std::mt19937 random_engine_;
    
    // Motion parameters
    void loadParams(const rclcpp::Node* node);
    double v_linear_max_;
    double w_angular_max_;
    double delta_t_;
    int num_samples_;
};

} // namespace avoa3d

#endif // AVOA3D_SAMPLE_GENERATOR_HPP