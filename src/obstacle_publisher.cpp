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
  : Node("obstacle_publisher"), start_time_(this->get_clock()->now()),
    have_obstacle_(false)  // Initialize to false until we receive obstacle data
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
    
    // Timer that calls timer_callback every 100 milliseconds
    timer_ = this->create_wall_timer(
      100ms, std::bind(&ObstaclePublisher::timer_callback, this));
      
    RCLCPP_INFO(this->get_logger(), "Waiting for obstacle data...");
  }

private:
  std::string fixed_frame_;
  std::string agent_frame_;
  bool have_obstacle_ = false;  // Flag to track if we have received obstacle data

  void odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    latest_odom_ = *msg;
    have_obstacle_ = true;  // Set to true when we receive obstacle data
  }

  void timer_callback()
  {
    // If we haven't received obstacle data yet, don't publish anything
    if (!have_obstacle_) {
      RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 2000, 
                          "Waiting for obstacle data...");
      return;
    }
    
    // Create an array message to hold the elements
    auto array_message = custom_msgs::msg::ElementCharacteristicsArray();
    
    // Create a single element message
    auto element = custom_msgs::msg::ElementCharacteristicsStamped();
    auto current_time = this->get_clock()->now();
    
    // Create a PointStamped for the obstacle position in world frame
    geometry_msgs::msg::PointStamped obstacle_world;
    obstacle_world.header.frame_id = fixed_frame_;
    obstacle_world.header.stamp = current_time;
    obstacle_world.point.x = latest_odom_.pose.pose.position.x;
    obstacle_world.point.y = latest_odom_.pose.pose.position.y;
    obstacle_world.point.z = latest_odom_.pose.pose.position.z;

    geometry_msgs::msg::PointStamped obstacle_agent_frame;

    try {
      rclcpp::Time now = this->get_clock()->now();
      
      // First, get the transform from world to agent frame
      geometry_msgs::msg::TransformStamped transform;
      transform = tf_buffer_->lookupTransform(
          agent_frame_,                   // target frame
          obstacle_world.header.frame_id,  // source frame
          now,                            // time
          50ms);    // timeout of 50ms 
      
      // Apply the transform to the obstacle point
      tf2::doTransform(obstacle_world, obstacle_agent_frame, transform);
      
      // Now obstacle_agent_frame.point has the coordinates in the agent's frame
      element.header.stamp = current_time;
      element.header.frame_id = agent_frame_;
      element.id = 1;
      element.type = 1;
      element.dynamic = false;
      
      // Use the transformed coordinates
      element.pose.position.x = obstacle_agent_frame.point.x;
      element.pose.position.y = obstacle_agent_frame.point.y;
      element.pose.position.z = obstacle_agent_frame.point.z;
      
      // Transform the orientation
      geometry_msgs::msg::QuaternionStamped orientation_world;
      orientation_world.header.frame_id = fixed_frame_;
      orientation_world.header.stamp = current_time;
      orientation_world.quaternion = latest_odom_.pose.pose.orientation;
      
      geometry_msgs::msg::QuaternionStamped orientation_agent;
      try {
        tf2::doTransform(orientation_world, orientation_agent, transform);
        element.pose.orientation = orientation_agent.quaternion;
      } catch (const tf2::TransformException & ex) {
        RCLCPP_WARN(this->get_logger(), "Orientation transform failed: %s", ex.what());
        // Fallback to original orientation
        element.pose.orientation = latest_odom_.pose.pose.orientation;
      }
      
      // Transform the velocity vector
      geometry_msgs::msg::Vector3Stamped vel_world;
      vel_world.header.frame_id = fixed_frame_;
      vel_world.header.stamp = current_time;
      vel_world.vector = latest_odom_.twist.twist.linear;
      
      geometry_msgs::msg::Vector3Stamped vel_agent;
      try {
        tf2::doTransform(vel_world, vel_agent, transform);
        // Set the obstacle's velocity in the agent's frame
        element.velocity.x = vel_agent.vector.x;
        element.velocity.y = vel_agent.vector.y;
        element.velocity.z = vel_agent.vector.z;
      } catch (const tf2::TransformException & ex) {
        RCLCPP_WARN(this->get_logger(), "Velocity transform failed: %s", ex.what());
        // Fallback to zeros
        element.velocity.x = 0.0;
        element.velocity.y = 0.0;
        element.velocity.z = 0.0;
      }
      
      // Set the size and protective zone
      element.size.x = 1.0;
      element.size.y = 1.0;
      element.size.z = 1.0;
      element.protective_zone = 0.0;
      
      array_message.elements.push_back(element);
      publisher_->publish(array_message);
      
    } catch (const tf2::TransformException &ex) {
      RCLCPP_ERROR(this->get_logger(), "Failed to transform obstacle to agent frame: %s", ex.what());
      // If the transform fails, don't publish anything or use fallback values if needed
      
      // Alternative: Use untransformed values as fallback
      element.header.stamp = current_time;
      element.header.frame_id = fixed_frame_;
      element.id = 1;
      element.type = 1;
      element.dynamic = false;
      
      // Use original coordinates
      element.pose.position.x = latest_odom_.pose.pose.position.x;
      element.pose.position.y = latest_odom_.pose.pose.position.y;
      element.pose.position.z = latest_odom_.pose.pose.position.z;
      element.pose.orientation = latest_odom_.pose.pose.orientation;
      
      // Use original velocity
      element.velocity.x = latest_odom_.twist.twist.linear.x;
      element.velocity.y = latest_odom_.twist.twist.linear.y;
      element.velocity.z = latest_odom_.twist.twist.linear.z;
      
      // Set the size and protective zone
      element.size.x = 1.0;
      element.size.y = 1.0;
      element.size.z = 1.0;
      element.protective_zone = 0.0;
      
      RCLCPP_WARN(this->get_logger(), "Using untransformed coordinates as fallback");
      
      array_message.elements.push_back(element);
      publisher_->publish(array_message);
    }
  }

  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;

  rclcpp::Publisher<custom_msgs::msg::ElementCharacteristicsArray>::SharedPtr publisher_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr obstacle_odometry_subscriber_;
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