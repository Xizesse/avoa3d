#include <chrono>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
#include "custom_msgs/msg/element_characteristics_stamped.hpp"
#include "custom_msgs/msg/element_characteristics_array.hpp"
#include "nav_msgs/msg/odometry.hpp"

#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <geometry_msgs/msg/point_stamped.hpp>

using namespace std::chrono_literals;

class ObstaclePublisher : public rclcpp::Node
{
public:
  ObstaclePublisher()
  : Node("obstacle_publisher"), start_time_(this->get_clock()->now())
  {
    // Declare and get frame ID parameters with defaults
    this->declare_parameter<std::string>("fixed_frame", "map");
    this->declare_parameter<std::string>("agent_frame", "agent");
    
    fixed_frame_ = this->get_parameter("fixed_frame").as_string();
    agent_frame_ = this->get_parameter("agent_frame").as_string();
    
    RCLCPP_INFO(this->get_logger(), "Using frames: fixed=%s, agent=%s", 
                fixed_frame_.c_str(), agent_frame_.c_str());

    tf_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

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
  std::string fixed_frame_;
  std::string agent_frame_;

  void odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    // Latest Odom
    latest_odom_ = *msg;  
  }

  void agent_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    agent_odometry_ = *msg;
  }

  void timer_callback()
{
  // Create an array message to hold the elements
  auto array_message = custom_msgs::msg::ElementCharacteristicsArray();
  
  // Create a single element message
  auto element = custom_msgs::msg::ElementCharacteristicsStamped();
  auto current_time = this->get_clock()->now();
  
  // Default values in case transform fails
  double obstacle_agent_x = latest_odom_.pose.pose.position.x;
  double obstacle_agent_y = latest_odom_.pose.pose.position.y;
  double obstacle_agent_z = latest_odom_.pose.pose.position.z;
  
  //!TRANSFROM COORDINATES
  // Create a PointStamped for the obstacle position in world frame
  geometry_msgs::msg::PointStamped obstacle_world;
  obstacle_world.header.frame_id = fixed_frame_;  // Use parameter instead of hardcoded "map"
  obstacle_world.header.stamp = this->get_clock()->now();
  obstacle_world.point.x = latest_odom_.pose.pose.position.x;
  obstacle_world.point.y = latest_odom_.pose.pose.position.y;
  obstacle_world.point.z = latest_odom_.pose.pose.position.z;  

  bool transform_success = false;
  try {
    geometry_msgs::msg::PointStamped obstacle_agent_frame;
    obstacle_agent_frame = tf_buffer_->transform(obstacle_world, agent_frame_);  // Use parameter
    
    // Now obstacle_agent_frame.point has the coordinates in the agent's frame
    obstacle_agent_x = obstacle_agent_frame.point.x;
    obstacle_agent_y = obstacle_agent_frame.point.y;
    obstacle_agent_z = obstacle_agent_frame.point.z;
    
    transform_success = true;
  }
  catch (const tf2::TransformException & ex) {
    RCLCPP_ERROR(this->get_logger(), "Transform failed: %s", ex.what());
  }
  //!TRANSFROM COORDINATES

  element.header.stamp = current_time;
  element.header.frame_id = agent_frame_;  // Use parameter
  element.id = 1;
  element.type = 1;
  element.dynamic = false;
  
  // Use transformed coordinates or fallback to original
  element.pose.position.x = obstacle_agent_x;
  element.pose.position.y = obstacle_agent_y;
  element.pose.position.z = obstacle_agent_z;
  
  if (!transform_success) {
    RCLCPP_WARN(this->get_logger(), "Using untransformed coordinates");
  }
  
  // You may also need to transform the orientation
  // This is a simplified approach - just using the original orientation
  element.pose.orientation = latest_odom_.pose.pose.orientation;
  
  // Convert VElocity Here
  geometry_msgs::msg::Vector3Stamped vel_world;
  vel_world.header.frame_id = fixed_frame_;  // Use parameter
  vel_world.header.stamp = current_time;

  vel_world.vector = latest_odom_.twist.twist.linear;

  try {
    // Transform the velocity vector to the agent's frame
    geometry_msgs::msg::Vector3Stamped vel_agent;
    vel_agent = tf_buffer_->transform(vel_world, agent_frame_);  // Use parameter
    
    // Set the obstacle's velocity in the agent's frame
    element.velocity.x = vel_agent.vector.x;
    element.velocity.y = vel_agent.vector.y;
    element.velocity.z = vel_agent.vector.z;
  }
  catch (const tf2::TransformException & ex) {
    RCLCPP_WARN(this->get_logger(), "Velocity transform failed: %s", ex.what());
    // Fallback to zeros
    element.velocity.x = 0.0;
    element.velocity.y = 0.0;
    element.velocity.z = 0.0;
  }


  element.size.x = 1.0;
  element.size.y = 1.0;
  element.size.z = 1.0;
  element.protective_zone = 0.0;  // Adding a small protective zone
  
  array_message.elements.push_back(element);
  
  publisher_->publish(array_message);
}

  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;

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