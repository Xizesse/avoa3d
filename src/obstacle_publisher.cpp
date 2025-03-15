/*
###
ros2 run avoa3d obstacle_publisher
###
*/
#include <chrono>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
#include "custom_msgs/msg/element_characteristics_stamped.hpp"
#include "custom_msgs/msg/element_characteristics_array.hpp"
#include "nav_msgs/msg/odometry.hpp"

using namespace std::chrono_literals;

class ObstaclePublisher : public rclcpp::Node
{
public:
  ObstaclePublisher()
  : Node("obstacle_publisher"), start_time_(this->get_clock()->now())
  {
    // Publisher for the array of obstacles
    publisher_ = this->create_publisher<custom_msgs::msg::ElementCharacteristicsArray>("/element_tracking/elements", 10);
    
    // Subscriber for the obstacle odometry
    obstacle_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/model/obstacle/odometry", 10,
      std::bind(&ObstaclePublisher::odometry_callback, this, std::placeholders::_1));
    agent_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/model/agente/odometry", 10,
      std::bind(&ObstaclePublisher::agent_odometry_callback, this, std::placeholders::_1));
    // Timer that calls timer_callback every 100 milliseconds
    timer_ = this->create_wall_timer(
      100ms, std::bind(&ObstaclePublisher::timer_callback, this));
  }

private:
  void odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    // Latest Odom
    latest_odom_ = *msg;  
  }

  void agent_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    // Latest agent odometry
    agent_odometry_ = *msg;
  }

  void timer_callback()
  {
    // Create an array message to hold the elements
    auto array_message = custom_msgs::msg::ElementCharacteristicsArray();
    
    // Create a single element message
    auto element = custom_msgs::msg::ElementCharacteristicsStamped();
    auto current_time = this->get_clock()->now();
    
    
    // Extract agent position and orientation
    double agent_x = agent_odometry_.pose.pose.position.x;
    double agent_y = agent_odometry_.pose.pose.position.y;
    
    // Extract quaternion
    double qx = agent_odometry_.pose.pose.orientation.x;
    double qy = agent_odometry_.pose.pose.orientation.y;
    double qz = agent_odometry_.pose.pose.orientation.z;
    double qw = agent_odometry_.pose.pose.orientation.w;
    
    // Convert quaternion to yaw (assuming 2D motion)
    double siny_cosp = 2.0 * (qw * qz + qx * qy);
    double cosy_cosp = 1.0 - 2.0 * (qy * qy + qz * qz);
    double agent_yaw = std::atan2(siny_cosp, cosy_cosp);
    
    // Get obstacle position in world frame
    double obstacle_world_x = latest_odom_.pose.pose.position.x;
    double obstacle_world_y = latest_odom_.pose.pose.position.y;
    
    // Transform to agent's frame
    // First translate relative to agent
    double dx = obstacle_world_x - agent_x;
    double dy = obstacle_world_y - agent_y;
    
    // Then rotate by negative agent yaw
    double obstacle_agent_x = dx * std::cos(-agent_yaw) - dy * std::sin(-agent_yaw);
    double obstacle_agent_y = dx * std::sin(-agent_yaw) + dy * std::cos(-agent_yaw);
    
    // Fill in the element details
    element.header.stamp = current_time;
    element.header.frame_id = "agent_frame"; // Change to your agent's frame id
    element.id = 1;
    element.type = 1;
    element.dynamic = false;
    
    // Use transformed coordinates
    element.pose.position.x = obstacle_agent_x;
    element.pose.position.y = obstacle_agent_y;
    element.pose.position.z = latest_odom_.pose.pose.position.z;
    
    // You may also need to transform the orientation
    // This is a simplified approach - just using the original orientation
    element.pose.orientation = latest_odom_.pose.pose.orientation;
    
    // Add velocity (would also need transformation for dynamic obstacles)
    element.velocity.x = 0.0;
    element.velocity.y = 0.0;
    element.velocity.z = 0.0;
    element.size.x = 1.0;
    element.size.y = 1.0;
    element.size.z = 1.0;
    element.protective_zone = 1.5;  // Adding a small protective zone
    
    // Add the element to the array
    array_message.elements.push_back(element);
    
    // Publish the array
    publisher_->publish(array_message);
  }

  rclcpp::Publisher<custom_msgs::msg::ElementCharacteristicsArray>::SharedPtr publisher_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr obstacle_odometry_subscriber_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr agent_odometry_subscriber_;
  nav_msgs::msg::Odometry agent_odometry_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Time start_time_;
  nav_msgs::msg::Odometry latest_odom_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ObstaclePublisher>());
  rclcpp::shutdown();
  return 0;
}