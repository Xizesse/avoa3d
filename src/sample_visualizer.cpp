// src/sample_visualizer.cpp
#include "avoa3d/sample_visualizer.hpp"
#include <sensor_msgs/point_cloud2_iterator.hpp>
#include <limits>

namespace avoa3d {

SampleVisualizer::SampleVisualizer(rclcpp::Node* node)
    : node_(node)
{
    // Get frame parameters from the node
    std::string agent_frame = node_->get_parameter("agent_frame").as_string();
    
    if (agent_frame.empty()) {
        agent_frame = "agent";
    }
    
    agent_frame_ = agent_frame;
    
    // Create publisher
    samples_cloud_publisher_ = node_->create_publisher<sensor_msgs::msg::PointCloud2>(
        "/avoa/velocity_samples", 1);
}

void SampleVisualizer::setAgentOdometry(const nav_msgs::msg::Odometry& odometry)
{
    latest_agent_odometry_ = odometry;
}

void SampleVisualizer::publishSamplesAsPointcloud(
    const std::vector<VelocitySample>& samples, 
    const VelocitySample& best_sample)
{
    // Create point cloud message
    sensor_msgs::msg::PointCloud2 cloud_msg;
    
    // Setup point cloud fields (10 fields total now)
    sensor_msgs::PointCloud2Modifier modifier(cloud_msg);
    modifier.setPointCloud2Fields(10,
        "x", 1, sensor_msgs::msg::PointField::FLOAT32,
        "y", 1, sensor_msgs::msg::PointField::FLOAT32,
        "z", 1, sensor_msgs::msg::PointField::FLOAT32,
        "cost", 1, sensor_msgs::msg::PointField::FLOAT32,
        "danger", 1, sensor_msgs::msg::PointField::FLOAT32,
        "heading", 1, sensor_msgs::msg::PointField::FLOAT32,
        "momentum", 1, sensor_msgs::msg::PointField::FLOAT32,
        "magnitude", 1, sensor_msgs::msg::PointField::FLOAT32,
        "linear_matching", 1, sensor_msgs::msg::PointField::FLOAT32,
        "twist_matching", 1, sensor_msgs::msg::PointField::FLOAT32);
    
    // Set point cloud metadata
    cloud_msg.header.frame_id = agent_frame_; 
    cloud_msg.header.stamp = node_->get_clock()->now();

    // Allocate point cloud
    modifier.resize(samples.size());
    
    // Create iterators
    sensor_msgs::PointCloud2Iterator<float> iter_x(cloud_msg, "x");
    sensor_msgs::PointCloud2Iterator<float> iter_y(cloud_msg, "y");
    sensor_msgs::PointCloud2Iterator<float> iter_z(cloud_msg, "z");
    sensor_msgs::PointCloud2Iterator<float> iter_cost(cloud_msg, "cost");
    sensor_msgs::PointCloud2Iterator<float> iter_danger(cloud_msg, "danger");
    sensor_msgs::PointCloud2Iterator<float> iter_heading(cloud_msg, "heading");
    sensor_msgs::PointCloud2Iterator<float> iter_momentum(cloud_msg, "momentum");
    sensor_msgs::PointCloud2Iterator<float> iter_magnitude(cloud_msg, "magnitude");
    sensor_msgs::PointCloud2Iterator<float> iter_linear(cloud_msg, "linear_matching");
    sensor_msgs::PointCloud2Iterator<float> iter_twist(cloud_msg, "twist_matching");
    
    // Add points to point cloud
    for (const auto& sample : samples) {
        *iter_x = sample.velocity_space.x;
        *iter_y = sample.velocity_space.y;
        *iter_z = sample.velocity_space.z;
        *iter_cost = sample.cost;
        *iter_danger = sample.danger;
        *iter_heading = sample.heading;
        *iter_momentum = sample.momentum;
        *iter_magnitude = sample.magnitude;
        *iter_linear = sample.linear_matching;
        *iter_twist = sample.twist_matching;

        ++iter_x; ++iter_y; ++iter_z; ++iter_cost; ++iter_danger;
        ++iter_heading; ++iter_momentum; ++iter_magnitude;
        ++iter_linear; ++iter_twist;
    }
    
    samples_cloud_publisher_->publish(cloud_msg);
}

} // namespace avoa3d