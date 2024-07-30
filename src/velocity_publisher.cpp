

/*
ros2 run avoa velocity_publisher
 */


#include <chrono>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

using namespace std::chrono_literals;

class VelocityPublisher : public rclcpp::Node
{
public:
  VelocityPublisher()
  : Node("velocity_publisher"), start_time_(this->get_clock()->now())
  {
    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/agent_vel_cmd", 10);
    timer_ = this->create_wall_timer(
      100ms, std::bind(&VelocityPublisher::timer_callback, this));
  }

private:
  void timer_callback()
  {
    auto msg = geometry_msgs::msg::Twist();
    auto current_time = this->get_clock()->now() - start_time_;
    double t = current_time.seconds();

    // Harmonic motion
    msg.linear.x = std::sin(t);
    msg.linear.y = std::cos(t);
    msg.linear.z = std::sin(t/2);

    RCLCPP_INFO(this->get_logger(), "Publishing: '%f', '%f', '%f'", msg.linear.x, msg.linear.y, msg.linear.z);
    publisher_->publish(msg);
  }

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
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