

/*
ros2 run avoa velocity_publisher
 */


#include <chrono>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
//#include "geometry_msgs/msg/twist.hpp"

#include "avoa/msg/element_characteristics_stamped.hpp"
//#include "avoa/msg/elementcharacteristicsarray.hpp"


using namespace std::chrono_literals;


class ObstaclePublisher : public rclcpp::Node
{
public:
  ObstaclePublisher()
  : Node("obstacle_publisher"), start_time_(this->get_clock()->now())
  {
    // Create a publisher for the custom message
    //publisher_ = this->create_publisher<avoa::msg::ElementCharacteristicsArray>("/obstacleArray", 10);
    publisher_ = this->create_publisher<avoa::msg::ElementCharacteristicsStamped>("/obstacleStamped", 10);
    
    // Create a timer that calls timer_callback every 100 milliseconds
    timer_ = this->create_wall_timer(
      100ms, std::bind(&ObstaclePublisher::timer_callback, this));
  }

private:

  void timer_callback()
  {
    //~auto message = avoa::msg::ElementCharacteristicsArray();
    auto message = avoa::msg::ElementCharacteristicsStamped();
    // TODO : Preencher a mensagem
    auto current_time = this->get_clock()->now();
    message.header.stamp = current_time;
    message.header.frame_id = "map";
    message.id = 1;
    message.type = 1;
    message.dynamic = false;
    message.pose.position.x = 1.0;
    message.pose.position.y = 2.0;
    message.pose.position.z = 0.0;
    message.pose.orientation.x = 0.0;
    message.pose.orientation.y = 0.0;
    message.pose.orientation.z = 0.0;
    message.pose.orientation.w = 1.0;
    message.size.x = 1.0;
    message.size.y = 1.0;
    message.size.z = 1.0;
    message.protective_zone = 0.0;


    publisher_->publish(message);
  }

  rclcpp::Publisher<avoa::msg::ElementCharacteristicsStamped>::SharedPtr publisher_;
  
  rclcpp::TimerBase::SharedPtr timer_;
  
  rclcpp::Time start_time_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ObstaclePublisher>());
  rclcpp::shutdown();
  return 0;
}