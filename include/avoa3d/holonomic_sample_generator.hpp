#ifndef AVOA3D_HOLONOMIC_SAMPLE_GENERATOR_HPP
#define AVOA3D_HOLONOMIC_SAMPLE_GENERATOR_HPP

#include "avoa3d/sample_generator.hpp"

namespace avoa3d
{

class HolonomicSampleGenerator : public SampleGenerator
{
public:
    HolonomicSampleGenerator(const rclcpp::Logger& logger);

    std::vector<VelocitySample> generateSamples(
        const geometry_msgs::msg::Twist& current_velocity,
        const geometry_msgs::msg::Twist& desired_velocity) override;
    
    geometry_msgs::msg::Twist translateToTwist(const VelocitySample& sample) override;
    
    void setParams(
        double v_x_max, double v_y_max, double v_z_max,
        double a_x_max, double a_y_max, double a_z_max,
        double w_roll_max, double w_pitch_max, double w_yaw_max,
        double a_roll_max, double a_pitch_max, double a_yaw_max,
        double delta_t, int num_samples) override;
    
private:
    rclcpp::Logger logger_;
    std::mt19937 random_engine_;
};

class HolonomicEllipsoidalSampleGenerator : public SampleGenerator
{
public:
    HolonomicEllipsoidalSampleGenerator(const rclcpp::Logger& logger);

    std::vector<VelocitySample> generateSamples(
        const geometry_msgs::msg::Twist& current_velocity,
        const geometry_msgs::msg::Twist& desired_velocity) override;
    
    geometry_msgs::msg::Twist translateToTwist(const VelocitySample& sample) override;
    
    void setParams(
        double v_x_max, double v_y_max, double v_z_max,
        double a_x_max, double a_y_max, double a_z_max,
        double w_roll_max, double w_pitch_max, double w_yaw_max,
        double a_roll_max, double a_pitch_max, double a_yaw_max,
        double delta_t, int num_samples) override;
    
private:
    rclcpp::Logger logger_;
    std::mt19937 random_engine_;
    
    bool isInsideEllipsoid(double vx, double vy, double vz, 
                          double vx_max, double vy_max, double vz_max) const;
    
    VelocitySample generateEllipsoidSample(std::mt19937& gen,
                                         double vx_max, double vy_max, double vz_max) const;
};

} // namespace avoa3d

#endif // AVOA3D_HOLONOMIC_SAMPLE_GENERATOR_HPP
