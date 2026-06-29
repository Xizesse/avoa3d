// include/s3vo/sample_visualizer.hpp
#ifndef S3VO_SAMPLE_VISUALIZER_HPP
#define S3VO_SAMPLE_VISUALIZER_HPP

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include "s3vo/velocity_sample.hpp"

namespace s3vo {

class SampleVisualizer {
public:
    SampleVisualizer(rclcpp::Node* node);
    
    void setAgentOdometry(const nav_msgs::msg::Odometry& odometry);
    
    void publishSamplesAsPointcloud(
        const std::vector<VelocitySample>& samples, 
        const VelocitySample& best_sample
    );
    
private:
    rclcpp::Node* node_;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr samples_cloud_publisher_;
    nav_msgs::msg::Odometry latest_agent_odometry_{};
    std::string agent_frame_;
};

} // namespace s3vo

#endif // S3VO_SAMPLE_VISUALIZER_HPP