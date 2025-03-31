#include <chrono>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/odometry.hpp"

#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <geometry_msgs/msg/point_stamped.hpp>


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
    tf_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

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
  
  const double constant_speed_ = 1.5; //! fixed speed
  
  void agent_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    current_x_ = msg->pose.pose.position.x;
    current_y_ = msg->pose.pose.position.y;
    current_z_ = msg->pose.pose.position.z;
  }
  
  void goal_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    //todo : Convert this to agent frame can be here

    goal_x_ = msg->pose.pose.position.x;
    goal_y_ = msg->pose.pose.position.y;
    goal_z_ = msg->pose.pose.position.z;

    
  }

  void timer_callback()
  {
    geometry_msgs::msg::PointStamped goal_in_world;
    goal_in_world.header.frame_id = "map";
    goal_in_world.header.stamp = this->get_clock()->now();
    goal_in_world.point.x = goal_x_;
    goal_in_world.point.y = goal_y_;
    goal_in_world.point.z = goal_z_;

    //std::cout << "Goal in world frame: " << goal_in_world.point.x << ", " << goal_in_world.point.y << ", " << goal_in_world.point.z << std::endl;

    geometry_msgs::msg::PointStamped goal_in_agent;
    std::string agent_frame_ = "agent";

    try {
      goal_in_agent = tf_buffer_->transform(goal_in_world, agent_frame_);

    } catch (const tf2::TransformException &ex) {
      RCLCPP_ERROR(this->get_logger(), "Failed to transform goal to agent frame: %s", ex.what());
      // If the transform fails, we can either return or just publish zero velocity
      geometry_msgs::msg::Twist zero_cmd;
      publisher_->publish(zero_cmd);
      return;
    }

    auto msg = geometry_msgs::msg::Twist();

    double error_x = goal_in_agent.point.x;
    double error_y = goal_in_agent.point.y;
    double error_z = goal_in_agent.point.z;

    //double distance = std::sqrt(error_x * error_x + error_y * error_y + error_z * error_z);
    double distance=std::sqrt(error_x * error_x + error_y*error_y);

    if (distance < 0.5) 
    {
      msg.linear.x = 0.0;
      msg.linear.y = 0.0;
      msg.linear.z = 0.0;
    }

    else if (distance < 3.0)
    {
      msg.linear.x = (error_x)/3.0 * constant_speed_ ;
      msg.linear.y = (error_y)/3.0 * constant_speed_ ;
      msg.linear.z = (error_z)/3.0 * constant_speed_ ;
    }

    else
    {
      // Normalize direction and apply constant speed
      msg.linear.x = (error_x / distance) * constant_speed_;
      msg.linear.y = (error_y / distance) * constant_speed_;
      msg.linear.z = (error_z / distance) * constant_speed_;
    }

    publisher_->publish(msg);
  }

  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;

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