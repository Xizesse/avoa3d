

/*
ros2 run avoa <>
 */

#include <chrono>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/odometry.hpp"

using namespace std::chrono_literals;

class VelocityPublisher : public rclcpp::Node
{
public:
  VelocityPublisher()
  : Node("velocity_publisher"), start_time_(this->get_clock()->now())
  {
    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/model/agente/cmd_vel", 10);
    goal_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/model/goal/cmd_vel", 10);

    timer_ = this->create_wall_timer(
      100ms, std::bind(&VelocityPublisher::timer_callback, this));

    // Subscribe to agent odometry to get current position
    agent_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/model/agente/odometry", 10, std::bind(&VelocityPublisher::agent_odometry_callback, this, std::placeholders::_1));
  }

private:
  // Goal coordinates
  double goal_x_ = 10.0;  
  double goal_y_ = 0.0; 
  double goal_z_ = 0.0;

  double current_x_ = 0.0;
  double current_y_ = 0.0;
  double current_z_ = 0.0;  

  
  void agent_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    current_x_ = msg->pose.pose.position.x;
    current_y_ = msg->pose.pose.position.y;
    current_z_ = msg->pose.pose.position.z;
  }

  void timer_callback()
  {
    auto msg = geometry_msgs::msg::Twist();

    double error_x = goal_x_ - current_x_;
    double error_y = goal_y_ - current_y_;
    double error_z = goal_z_ - current_z_;

    double Kp = 0.5;
    msg.linear.x = Kp * error_x;
    msg.linear.y = Kp * error_y;
    msg.linear.z = Kp * error_z;

    RCLCPP_INFO(this->get_logger(), "Publishing: '%f', '%f', '%f'", msg.linear.x, msg.linear.y, msg.linear.z);
    publisher_->publish(msg);

    auto goal_msg = geometry_msgs::msg::Twist();
    goal_msg.linear.x = 0.0;
    goal_msg.linear.y = 0.0;
    goal_msg.linear.z = 0.0;

    goal_publisher_->publish(goal_msg);
  }

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr goal_publisher_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr agent_odometry_subscriber_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Time start_time_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<VelocityPublisher>());
  rclcpp::shutdown();
  return 0;
}