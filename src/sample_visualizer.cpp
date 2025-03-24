// src/sample_visualizer.cpp
#include "avoa3d/sample_visualizer.hpp"
#include <sensor_msgs/point_cloud2_iterator.hpp>
#include <limits>

namespace avoa3d {

SampleVisualizer::SampleVisualizer(rclcpp::Node* node, double delta_t)
    : node_(node),
      delta_t_(delta_t)
{
    // Create publisher
    samples_cloud_publisher_ = node_->create_publisher<sensor_msgs::msg::PointCloud2>(
        "/avoa/velocity_samples", 10);
}

void SampleVisualizer::setAgentOdometry(const nav_msgs::msg::Odometry& odometry)
{
    latest_agent_odometry_ = odometry;
}

void SampleVisualizer::publishSamplesAsPointcloud(
    const std::vector<VelocitySample>& samples, 
    const VelocitySample& best_sample)
{
    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::lowest();

    // Create point cloud message
    sensor_msgs::msg::PointCloud2 cloud_msg;
    
    // Setup point cloud fields
    sensor_msgs::PointCloud2Modifier modifier(cloud_msg);
    modifier.setPointCloud2Fields(5,
        "x", 1, sensor_msgs::msg::PointField::FLOAT32,
        "y", 1, sensor_msgs::msg::PointField::FLOAT32,
        "z", 1, sensor_msgs::msg::PointField::FLOAT32,
        "cost", 1, sensor_msgs::msg::PointField::FLOAT32,
        "danger", 1, sensor_msgs::msg::PointField::FLOAT32);
    
    // Set point cloud metadata
    cloud_msg.header.frame_id = "agent";
    cloud_msg.header.stamp = node_->now();
    
    // Allocate point cloud
    modifier.resize(samples.size());
    
    // Create iterators
    sensor_msgs::PointCloud2Iterator<float> iter_x(cloud_msg, "x");
    sensor_msgs::PointCloud2Iterator<float> iter_y(cloud_msg, "y");
    sensor_msgs::PointCloud2Iterator<float> iter_z(cloud_msg, "z");
    sensor_msgs::PointCloud2Iterator<float> iter_cost(cloud_msg, "cost");
    sensor_msgs::PointCloud2Iterator<float> iter_danger(cloud_msg, "danger");

    
    // Add points to point cloud
    for (const auto& sample : samples) {
        *iter_x = sample.vx ;
        *iter_y =  sample.vy ;
        *iter_z = sample.vz ;
        *iter_cost = sample.cost;
        *iter_danger = sample.danger;
        
        
        double pos_x = sample.vx ;
        double pos_y = sample.vy ;
        min_x = std::min(min_x, pos_x);
        max_x = std::max(max_x, pos_x);
        min_y = std::min(min_y, pos_y);
        max_y = std::max(max_y, pos_y);

        ++iter_x;
        ++iter_y;
        ++iter_z;
        ++iter_cost;
        ++iter_danger;
    }
    
    samples_cloud_publisher_->publish(cloud_msg);
}

} // namespace avoa3d