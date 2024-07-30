#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/marker.hpp"
#include "geometry_msgs/msg/twist.hpp"

using namespace std::chrono_literals;

class MarkerPublisher : public rclcpp::Node
{
public:
  MarkerPublisher()
  : Node("marker_publisher"), count_(0)
  {
    publisher_ = this->create_publisher<visualization_msgs::msg::Marker>("visualization_marker", 10);
    velocity_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
      "/model/xico/cmd_vel", 10, std::bind(&MarkerPublisher::velocity_callback, this, std::placeholders::_1));
    timer_ = this->create_wall_timer(100ms, std::bind(&MarkerPublisher::timer_callback, this));
  }

private:
  void timer_callback()
  {
    //! AGENT MARKER
    auto marker_msg = visualization_msgs::msg::Marker();
    marker_msg.header.frame_id = "map";  // Replace with your desired frame_id
    marker_msg.header.stamp = this->now();
    marker_msg.ns = "basic_shapes";
    marker_msg.id = 0;
    marker_msg.type = visualization_msgs::msg::Marker::SPHERE;
    marker_msg.action = visualization_msgs::msg::Marker::ADD;
    marker_msg.pose.position.x = 0.0;
    marker_msg.pose.position.y = 0.0;
    marker_msg.pose.position.z = 0.0;
    marker_msg.pose.orientation.x = 0.0;
    marker_msg.pose.orientation.y = 0.0;
    marker_msg.pose.orientation.z = 0.0;
    marker_msg.pose.orientation.w = 1.0;
    marker_msg.scale.x = 1.0;
    marker_msg.scale.y = 1.0;
    marker_msg.scale.z = 1.0;
    marker_msg.color.r = 1.0f;
    marker_msg.color.g = 0.0f;
    marker_msg.color.b = 1.0f;
    marker_msg.color.a = 0.3;
    marker_msg.lifetime = rclcpp::Duration(std::chrono::nanoseconds(static_cast<int64_t>(1e9))); 

    publisher_->publish(marker_msg);

    //! AGENT VELOCITY MARKER
    auto arrow_marker = visualization_msgs::msg::Marker();
    arrow_marker.header.frame_id = "map";  // Replace with your desired frame_id
    arrow_marker.header.stamp = this->now();
    arrow_marker.ns = "basic_shapes";
    arrow_marker.id = 1;  
    arrow_marker.type = visualization_msgs::msg::Marker::ARROW;
    arrow_marker.action = visualization_msgs::msg::Marker::ADD;
    arrow_marker.pose.position.x = 0.0;
    arrow_marker.pose.position.y = 0.0;
    arrow_marker.pose.position.z = 0.0;
    arrow_marker.pose.orientation.x = velocity_.linear.x;  // Arrow direction proportional to linear velocity
    arrow_marker.pose.orientation.y = velocity_.linear.y;
    arrow_marker.pose.orientation.z = velocity_.linear.z;
    arrow_marker.pose.orientation.w = 1.0;
    //proporcional to the norm of the linear velocity
    arrow_marker.scale.x = sqrt(pow(velocity_.linear.x, 2) + pow(velocity_.linear.y, 2) + pow(velocity_.linear.z, 2));
    arrow_marker.scale.y = 0.1;  
    arrow_marker.scale.z = 0.1;  
    arrow_marker.color.r = 0.0f;
    arrow_marker.color.g = 1.0f;
    arrow_marker.color.b = 0.0f;
    arrow_marker.color.a = 1.0;
    arrow_marker.lifetime = rclcpp::Duration(std::chrono::nanoseconds(static_cast<int64_t>(1e9))); 

    publisher_->publish(arrow_marker);

    RCLCPP_INFO(this->get_logger(), "Publishing Agent Marker");
  }

  void velocity_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
  {
    velocity_ = *msg;
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr publisher_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr velocity_subscriber_;
  geometry_msgs::msg::Twist velocity_;
  size_t count_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MarkerPublisher>());
  rclcpp::shutdown();
  return 0;
}