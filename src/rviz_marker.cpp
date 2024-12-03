#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/marker.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Vector3.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include "geometry_msgs/msg/transform_stamped.hpp"

using namespace std::chrono_literals;

class MarkerPublisher : public rclcpp::Node
{
public:
  MarkerPublisher()
  : Node("marker_publisher"), count_(0)
  {
    tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);
    
    agent_publisher_ = this->create_publisher<visualization_msgs::msg::Marker>("agent_marker", 10);
    obstacle_publisher_ = this->create_publisher<visualization_msgs::msg::Marker>("obstacle_marker", 10);

    velocity_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
      "/model/agente/cmd_vel", 10, std::bind(&MarkerPublisher::velocity_callback, this, std::placeholders::_1));
    agent_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/model/agente/odometry", 10, std::bind(&MarkerPublisher::agent_odometry_callback, this, std::placeholders::_1));
    obstacle_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/model/obstacle/odometry", 10, std::bind(&MarkerPublisher::obstacle_odometry_callback, this, std::placeholders::_1)); 
    timer_ = this->create_wall_timer(100ms, std::bind(&MarkerPublisher::timer_callback, this));
  }

private:
  void timer_callback()
  {
    //! AGENT MARKER
    auto agent_marker_msg = visualization_msgs::msg::Marker();
    agent_marker_msg.header.frame_id = "map";
    agent_marker_msg.header.stamp = this->now();
    agent_marker_msg.ns = "basic_shapes";
    agent_marker_msg.id = 0;
    agent_marker_msg.type = visualization_msgs::msg::Marker::CUBE;
    agent_marker_msg.action = visualization_msgs::msg::Marker::ADD;
    agent_marker_msg.pose.position.x = agent_odometry_.pose.pose.position.x;
    agent_marker_msg.pose.position.y = agent_odometry_.pose.pose.position.y;
    agent_marker_msg.pose.position.z = agent_odometry_.pose.pose.position.z;
    agent_marker_msg.pose.orientation = agent_odometry_.pose.pose.orientation;
    agent_marker_msg.scale.x = 1.0;
    agent_marker_msg.scale.y = 1.0;
    agent_marker_msg.scale.z = 1.0;
    agent_marker_msg.color.r = 1.0f;
    agent_marker_msg.color.g = 0.0f;
    agent_marker_msg.color.b = 1.0f;
    agent_marker_msg.color.a = 0.3;
    agent_marker_msg.lifetime = rclcpp::Duration(std::chrono::nanoseconds(static_cast<int64_t>(1e9))); 

    agent_publisher_->publish(agent_marker_msg);

    //! OBSTACLE MARKER
    auto obstacle_marker_msg = visualization_msgs::msg::Marker();
    obstacle_marker_msg.header.frame_id = "map"; 
    obstacle_marker_msg.header.stamp = this->now();
    obstacle_marker_msg.ns = "basic_shapes";
    obstacle_marker_msg.id = 0;
    obstacle_marker_msg.type = visualization_msgs::msg::Marker::SPHERE;
    obstacle_marker_msg.action = visualization_msgs::msg::Marker::ADD;

    obstacle_marker_msg.pose.position.x = obstacle_odometry_.pose.pose.position.x;
    obstacle_marker_msg.pose.position.y = obstacle_odometry_.pose.pose.position.y;
    obstacle_marker_msg.pose.position.z = obstacle_odometry_.pose.pose.position.z;
    obstacle_marker_msg.pose.orientation = obstacle_odometry_.pose.pose.orientation;
    obstacle_marker_msg.scale.x = 1.0;
    obstacle_marker_msg.scale.y = 1.0;
    obstacle_marker_msg.scale.z = 1.0;
    obstacle_marker_msg.color.r = 0.0f;
    obstacle_marker_msg.color.g = 1.0f;
    obstacle_marker_msg.color.b = 0.0f;
    obstacle_marker_msg.color.a = 0.9;
    obstacle_marker_msg.lifetime = rclcpp::Duration(std::chrono::nanoseconds(static_cast<int64_t>(1e9))); 

    obstacle_publisher_->publish(obstacle_marker_msg);

    //! AGENT GIVEN VELOCITY MARKER
    auto arrow_marker = visualization_msgs::msg::Marker();
    arrow_marker.header.frame_id = "map";
    arrow_marker.header.stamp = this->now();
    arrow_marker.ns = "basic_shapes";
    arrow_marker.id = 1;
    arrow_marker.type = visualization_msgs::msg::Marker::ARROW;
    arrow_marker.action = visualization_msgs::msg::Marker::ADD;

    arrow_marker.pose.position.x = agent_odometry_.pose.pose.position.x;
    arrow_marker.pose.position.y = agent_odometry_.pose.pose.position.y;
    arrow_marker.pose.position.z = agent_odometry_.pose.pose.position.z;

    double vx = velocity_.linear.x;
    double vy = velocity_.linear.y;
    double vz = velocity_.linear.z;

    tf2::Vector3 velocity_vector(vx, vy, vz);
    tf2::Vector3 marker_direction = velocity_vector.normalized();

    tf2::Quaternion q;
    q.setRPY(0, 0, atan2(marker_direction.y(), marker_direction.x()));

    arrow_marker.pose.orientation.x = q.x();
    arrow_marker.pose.orientation.y = q.y();
    arrow_marker.pose.orientation.z = q.z();
    arrow_marker.pose.orientation.w = q.w();

    double velocity_magnitude = sqrt(vx * vx + vy * vy + vz * vz);
    arrow_marker.scale.x = velocity_magnitude;
    arrow_marker.scale.y = 0.1; 
    arrow_marker.scale.z = 0.1; 

    
    arrow_marker.color.r = 0.0f;
    arrow_marker.color.g = 1.0f;
    arrow_marker.color.b = 0.0f;
    arrow_marker.color.a = 1.0f;

    arrow_marker.lifetime = rclcpp::Duration::from_seconds(1);

    agent_publisher_->publish(arrow_marker);
    RCLCPP_INFO(this->get_logger(), "Publishing Markers");
    RCLCPP_INFO(this->get_logger(), "Publishing Markers");
  }

  void velocity_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
  {
    velocity_ = *msg;
  }

  void agent_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    agent_odometry_ = *msg;
    geometry_msgs::msg::TransformStamped transform_stamped;

    transform_stamped.header.stamp = this->get_clock()->now();
    transform_stamped.header.frame_id = "map";  // Parent frame
    transform_stamped.child_frame_id = "agent";  // Child frame

    transform_stamped.transform.translation.x = agent_odometry_.pose.pose.position.x;
    transform_stamped.transform.translation.y = agent_odometry_.pose.pose.position.y;
    transform_stamped.transform.translation.z = agent_odometry_.pose.pose.position.z;

    transform_stamped.transform.rotation = agent_odometry_.pose.pose.orientation;

    tf_broadcaster_->sendTransform(transform_stamped);
  } 

  void obstacle_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    obstacle_odometry_ = *msg;
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr agent_publisher_;
  rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr obstacle_publisher_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr velocity_subscriber_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr agent_odometry_subscriber_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr obstacle_odometry_subscriber_;
  geometry_msgs::msg::Twist velocity_;
  nav_msgs::msg::Odometry agent_odometry_;
  nav_msgs::msg::Odometry obstacle_odometry_;
  size_t count_;

  std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;  
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MarkerPublisher>());
  rclcpp::shutdown();
  return 0;
}
