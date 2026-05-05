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
    virtual ~SampleGenerator() = default;
    
    virtual std::vector<VelocitySample> generateSamples(
        const geometry_msgs::msg::Twist& current_velocity,
        const geometry_msgs::msg::Twist& desired_velocity) = 0;
    
    virtual geometry_msgs::msg::Twist translateToTwist(const VelocitySample& sample) = 0;
    
    virtual void setParams(
        double v_x_max, double v_y_max, double v_z_max,
        double a_x_max, double a_y_max, double a_z_max,
        double w_roll_max, double w_pitch_max, double w_yaw_max,
        double a_roll_max, double a_pitch_max, double a_yaw_max,
        double delta_t, int num_samples) = 0;
    
protected:
    SampleGenerator() = default;
    MotionParameters params_;
};

} // namespace avoa3d

// Include specialized generators
#include "avoa3d/holonomic_sample_generator.hpp"
#include "avoa3d/diff_drive_sample_generator.hpp"

#endif // AVOA3D_SAMPLE_GENERATOR_HPP