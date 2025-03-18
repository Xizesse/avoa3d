// include/avoa3d/sample_visualizer.hpp
#ifndef AVOA3D_SAMPLE_VISUALIZER_HPP
#define AVOA3D_SAMPLE_VISUALIZER_HPP

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include "avoa3d/velocity_sample.hpp"

namespace avoa3d {

class SampleVisualizer {
public:
    SampleVisualizer(rclcpp::Node* node, double delta_t);
    
    void setAgentOdometry(const nav_msgs::msg::Odometry& odometry);
    
    void publishSamplesAsPointcloud(
        const std::vector<VelocitySample>& samples, 
        const VelocitySample& best_sample
    );
    
private:
    rclcpp::Node* node_;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr samples_cloud_publisher_;
    nav_msgs::msg::Odometry latest_agent_odometry_{};
    double delta_t_;
};

} // namespace avoa3d

#endif // AVOA3D_SAMPLE_VISUALIZER_HPP