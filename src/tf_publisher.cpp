#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include <tf2/LinearMath/Quaternion.h>
#include "geometry_msgs/msg/transform_stamped.hpp"

using namespace std::chrono_literals;

class DynamicTfPublisher : public rclcpp::Node
{
public:
  DynamicTfPublisher()
  : Node("tf_publisher")  // Keep same name for compatibility
  {
    // Only need dynamic transform broadcaster
    tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);
    
    // Declare configurable parameter
    this->declare_parameter("agent_odometry_topic", "/model/agente/odometry");
    std::string odom_topic = this->get_parameter("agent_odometry_topic").as_string();
    
    // Subscribe to agent odometry - publish transform on every message
    agent_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      odom_topic, 10, 
      std::bind(&DynamicTfPublisher::agent_odometry_callback, this, std::placeholders::_1));
    
    RCLCPP_INFO(this->get_logger(), "Dynamic TF Publisher initialized");
    RCLCPP_INFO(this->get_logger(), "Subscribing to: %s", odom_topic.c_str());
    RCLCPP_INFO(this->get_logger(), "Publishing odom->base_link transform on odometry messages");
  }

private:
  void agent_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    geometry_msgs::msg::TransformStamped odom_to_base_link;
    odom_to_base_link.header.stamp = this->get_clock()->now();
    odom_to_base_link.header.frame_id = "odom";
    odom_to_base_link.child_frame_id = "base_link";
    
    // Copy pose from odometry message
    odom_to_base_link.transform.translation.x = msg->pose.pose.position.x;
    odom_to_base_link.transform.translation.y = msg->pose.pose.position.y;
    odom_to_base_link.transform.translation.z = msg->pose.pose.position.z;
    odom_to_base_link.transform.rotation = msg->pose.pose.orientation;
    
    // Publish transform immediately
    tf_broadcaster_->sendTransform(odom_to_base_link);
  }

  // Members
  std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr agent_odometry_subscriber_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DynamicTfPublisher>());
  rclcpp::shutdown();
  return 0;
}