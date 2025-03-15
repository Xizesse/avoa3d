#include <chrono>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/odometry.hpp"

using namespace std::chrono_literals;

template <typename T> T sgn(T val){ return (T(0) < val) - (val < T(0)); }

float normalizeAngle(float angle)
{
  float sign = sgn(angle);
  angle = fabs(angle);

  while(angle > M_PI)
    angle -= 2*M_PI;

  angle = sign*angle;
  return angle;
}

class VelocityPublisher : public rclcpp::Node
{
public:
  VelocityPublisher()
  : Node("velocity_publisher"), start_time_(this->get_clock()->now())
  {
    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/model/agente/desired_vel", 10);

    timer_ = this->create_wall_timer(
      100ms, std::bind(&VelocityPublisher::timer_callback, this));

    agent_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/model/agente/odometry", 10, std::bind(&VelocityPublisher::agent_odometry_callback, this, std::placeholders::_1));
    goal_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/model/goal/odometry", 10, std::bind(&VelocityPublisher::goal_odometry_callback, this, std::placeholders::_1));
  }

private:
  
  double goal_x_ = 0.0;
  double goal_y_ = 0.0;
  double goal_z_ = 0.0;

  
  double current_x_ = 0.0;
  double current_y_ = 0.0;
  double current_z_ = 0.0;
  
  const double constant_speed_ = 0.5; //! fixed speed
  
  void agent_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    current_x_ = msg->pose.pose.position.x;
    current_y_ = msg->pose.pose.position.y;
    current_z_ = msg->pose.pose.position.z;
  }
  
  void goal_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    goal_x_ = msg->pose.pose.position.x;
    goal_y_ = msg->pose.pose.position.y;
    goal_z_ = msg->pose.pose.position.z;
  }

  void timer_callback()
  {
    auto msg = geometry_msgs::msg::Twist();

    double error_x = goal_x_ - current_x_;
    double error_y = goal_y_ - current_y_;
    double error_z = goal_z_ - current_z_;

    //double distance = std::sqrt(error_x * error_x + error_y * error_y + error_z * error_z);
    double distance=std::sqrt(error_x * error_x + error_y*error_y);

    if (distance < 0.1) 
    {
      msg.linear.x = 0.0;
      msg.linear.y = 0.0;
      msg.linear.z = 0.0;
    }

    else
    {
      // Normalize direction and apply constant speed
      msg.linear.x = (error_x / distance) * constant_speed_;
      msg.linear.y = (error_y / distance) * constant_speed_;
      msg.linear.z = (error_z / distance) * constant_speed_;
      //msg.linear.z = 0;
   
    }

    publisher_->publish(msg);
  }

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr agent_odometry_subscriber_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr goal_odometry_subscriber_;
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