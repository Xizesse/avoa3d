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

//! /////////////////////////////
//! Diff_Drive_Velocity Generator
//! /////////////////////////////

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
  : Node("velocity_publisher")
  {
    // Initialize TF buffer and listener for frame transformations
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
  // Member variables declaration
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr agent_odometry_subscriber_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr goal_odometry_subscriber_;
  rclcpp::TimerBase::SharedPtr timer_;
  
  // Goal position in world frame
  double goal_x_ = 0.0;
  double goal_y_ = 0.0;
  double goal_z_ = 0.0;

  // Goal position in agent frame
  double goal_agent_x_ = 0.0;
  double goal_agent_y_ = 0.0;
  double goal_agent_z_ = 0.0;
  
  // Current position of agent
  double current_x_ = 0.0;
  double current_y_ = 0.0;
  double current_z_ = 0.0;
  
  const double constant_speed_ = 1.0; //! fixed speed
  
  // Store latest odometry messages
  nav_msgs::msg::Odometry latest_agent_odom_;
  nav_msgs::msg::Odometry latest_goal_odom_;
  bool agent_odom_received_ = false;
  bool goal_odom_received_ = false;
  
  void agent_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    latest_agent_odom_ = *msg;
    current_x_ = msg->pose.pose.position.x;
    current_y_ = msg->pose.pose.position.y;
    current_z_ = msg->pose.pose.position.z;
    agent_odom_received_ = true;
  }
  
  void goal_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    latest_goal_odom_ = *msg;
    goal_x_ = msg->pose.pose.position.x;
    goal_y_ = msg->pose.pose.position.y;
    goal_z_ = msg->pose.pose.position.z;
    goal_odom_received_ = true;
    transform_goal_to_agent_frame();
  }
  
  void transform_goal_to_agent_frame()
  {
    if (!agent_odom_received_ || !goal_odom_received_) {
      RCLCPP_WARN(this->get_logger(), "Cannot transform: Missing agent or goal odometry data");
      return;
    }
    
    geometry_msgs::msg::PointStamped goal_world;
    goal_world.header.frame_id = "map";  
    goal_world.header.stamp = this->get_clock()->now();
    goal_world.point.x = goal_x_;
    goal_world.point.y = goal_y_;
    goal_world.point.z = goal_z_;
    
    std::string agent_frame = "agent"; 
    
    try {
      geometry_msgs::msg::PointStamped goal_agent_frame;
      goal_agent_frame = tf_buffer_->transform(goal_world, agent_frame);
      
      goal_agent_x_ = goal_agent_frame.point.x;
      goal_agent_y_ = goal_agent_frame.point.y;
      goal_agent_z_ = goal_agent_frame.point.z;
      
      RCLCPP_DEBUG(this->get_logger(), "Goal in agent frame: [%f, %f, %f]",
                  goal_agent_x_, goal_agent_y_, goal_agent_z_);
    }
    catch (const tf2::TransformException & ex) {
      RCLCPP_ERROR(this->get_logger(), "Transform failed: %s", ex.what());
    }
  }

  void timer_callback()
  {
    auto msg = geometry_msgs::msg::Twist();
    
    transform_goal_to_agent_frame();
    
    if (agent_odom_received_ && goal_odom_received_) {
      double distance_squared = 
        goal_agent_x_ * goal_agent_x_ + 
        goal_agent_y_ * goal_agent_y_ + 
        goal_agent_z_ * goal_agent_z_;
      
      if (distance_squared < 0.1) 
      {
        // Close to goal, stop
        msg.linear.x = 0.0;
        msg.linear.y = 0.0;
        msg.linear.z = 0.0;
        msg.angular.x = 0.0;
        msg.angular.y = 0.0;
        msg.angular.z = 0.0;
        RCLCPP_INFO(this->get_logger(), "Goal reached, stopping");
      }
      else
      {
        // Fixed linear velocity
        msg.linear.x = 1.0;
        
        // Protect against division by zero or very small values
        // which could cause infinite or very large angular velocities
        if (std::abs(goal_agent_y_) > 0.1) {
          // Calculate curvature for pure pursuit
          double curvature = distance_squared / (2.0 * std::abs(goal_agent_y_));
          
          // Angular velocity is linear velocity divided by the turning radius
          // We need to determine the sign based on which side the goal is
          msg.angular.z = (goal_agent_y_ > 0) ? msg.linear.x / curvature : -msg.linear.x / curvature;
          
          // Limit angular velocity to prevent extreme rotation
          const double max_angular_velocity = 1.0; // rad/s
          if (std::abs(msg.angular.z) > max_angular_velocity) {
            msg.angular.z = std::copysign(max_angular_velocity, msg.angular.z);
          }
        } else {
          // If goal is directly ahead or behind, don't rotate
          msg.angular.z = 0.0;
        }
        
        RCLCPP_INFO(this->get_logger(), "Linear velocity: %f, Angular velocity: %f, goal_y: %f", 
                    msg.linear.x, msg.angular.z, goal_agent_y_);
      }
    } else {
      // If we don't have odometry data yet, send zero velocities
      msg.linear.x = 0.0;
      msg.angular.z = 0.0;
      RCLCPP_WARN(this->get_logger(), "Waiting for odometry data...");
    }
    
    publisher_->publish(msg);
  }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<VelocityPublisher>());
  rclcpp::shutdown();
  return 0;
}