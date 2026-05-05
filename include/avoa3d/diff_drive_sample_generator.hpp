#ifndef AVOA3D_DIFF_DRIVE_SAMPLE_GENERATOR_HPP
#define AVOA3D_DIFF_DRIVE_SAMPLE_GENERATOR_HPP

#include "avoa3d/sample_generator.hpp"

namespace avoa3d
{

class DiffDriveSampleGenerator : public SampleGenerator
{
public:
    DiffDriveSampleGenerator(const rclcpp::Logger& logger);
    DiffDriveSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node);
    
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
    
    double normalizeAngle(double angle);

private:
    rclcpp::Logger logger_;
    std::mt19937 random_engine_;
};

} // namespace avoa3d

#endif // AVOA3D_DIFF_DRIVE_SAMPLE_GENERATOR_HPP
