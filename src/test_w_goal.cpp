#include <chrono>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"  // For RViz2 goal pose

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
  : Node("velocity_publisher"), start_time_(this->get_clock()->now()),
    have_goal_(false)  // Initialize to false since we don't have a goal yet
  {
    // Declare and get frame ID parameters with defaults
    this->declare_parameter<std::string>("fixed_frame", "map");
    this->declare_parameter<std::string>("agent_frame", "agent");
    
    // Declare parameter for goal topic
    this->declare_parameter<std::string>("goal_topic", "/goal_pose");
    
    fixed_frame_ = this->get_parameter("fixed_frame").as_string();
    agent_frame_ = this->get_parameter("agent_frame").as_string();
    std::string goal_topic = this->get_parameter("goal_topic").as_string();
    
    RCLCPP_INFO(this->get_logger(), "Using frames: fixed=%s, agent=%s", 
                fixed_frame_.c_str(), agent_frame_.c_str());
    RCLCPP_INFO(this->get_logger(), "Subscribing to goal topic: %s", 
                goal_topic.c_str());

    tf_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/model/agente/desired_vel", 10);

    timer_ = this->create_wall_timer(
      100ms, std::bind(&VelocityPublisher::timer_callback, this));

    agent_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/model/agente/odometry", 10, 
      std::bind(&VelocityPublisher::agent_odometry_callback, this, std::placeholders::_1));
    
    // Subscribe to RViz2 goal pose topic
    goal_subscriber_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
      goal_topic, 10,
      std::bind(&VelocityPublisher::goal_callback, this, std::placeholders::_1));
    
    // Set default goal for fallback, but we'll wait for RViz2 goal
    goal_x_ = 0.0;
    goal_y_ = 0.0;
    goal_z_ = 0.0;
    
    RCLCPP_INFO(this->get_logger(), "Waiting for RViz2 goal...");
  }

private:
  std::string fixed_frame_;
  std::string agent_frame_;

  double goal_x_ = 0.0;  // Goal X position
  double goal_y_ = 0.0;  // Goal Y position
  double goal_z_ = 0.0;  // Goal Z position
  std::string goal_frame_ = "";  // Frame ID of the received goal
  bool have_goal_ = false;  // Flag to track if we have received a goal

  double current_x_ = 0.0;
  double current_y_ = 0.0;
  double current_z_ = 0.0;
  
  const double constant_speed_ = 1.0; //! fixed speed
  
  void agent_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    current_x_ = msg->pose.pose.position.x;
    current_y_ = msg->pose.pose.position.y;
    current_z_ = msg->pose.pose.position.z;
  }
  
  void goal_callback(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
  {
    // Update goal coordinates from RViz2
    goal_x_ = msg->pose.position.x;
    goal_y_ = msg->pose.position.y;
    goal_z_ = msg->pose.position.z;
    goal_frame_ = msg->header.frame_id;
    have_goal_ = true;
    
    RCLCPP_INFO(this->get_logger(), "Received new goal: [%.2f, %.2f, %.2f] in frame %s", 
                goal_x_, goal_y_, goal_z_, goal_frame_.c_str());
  }

  void timer_callback()
  {
    // If we haven't received a goal yet, just publish zero velocity
    if (!have_goal_) {
      auto zero_cmd = geometry_msgs::msg::Twist();
      publisher_->publish(zero_cmd);
      return;
    }
    
    geometry_msgs::msg::PointStamped goal_in_world;
    
    // Use the frame from the received goal message, or fall back to fixed_frame_
    if (!goal_frame_.empty()) {
      goal_in_world.header.frame_id = goal_frame_;
    } else {
      goal_in_world.header.frame_id = fixed_frame_;
    }
    
    goal_in_world.header.stamp = this->get_clock()->now();
    goal_in_world.point.x = goal_x_;
    goal_in_world.point.y = goal_y_;
    goal_in_world.point.z = goal_z_;

    geometry_msgs::msg::PointStamped goal_in_agent;

    try {
      rclcpp::Time now = this->get_clock()->now();
      
      // First, get the transform from world to agent frame
      geometry_msgs::msg::TransformStamped transform;
      transform = tf_buffer_->lookupTransform(
          agent_frame_,                   // target frame
          goal_in_world.header.frame_id,  // source frame
          now,                            // time
          50ms);    // timeout of 50ms 
      
      // Apply the transform to the goal point
      tf2::doTransform(goal_in_world, goal_in_agent, transform);
      
    } catch (const tf2::TransformException &ex) {
      RCLCPP_ERROR(this->get_logger(), "Failed to transform goal to agent frame: %s", ex.what());
      // If the transform fails, publish zero velocity
      geometry_msgs::msg::Twist zero_cmd;
      publisher_->publish(zero_cmd);
      return;
    }

    auto msg = geometry_msgs::msg::Twist();

    double error_x = goal_in_agent.point.x;
    double error_y = goal_in_agent.point.y;
    double error_z = goal_in_agent.point.z;

    double distance = std::sqrt(error_x * error_x + error_y * error_y);

    if (distance < 0.5) 
    {
      // Goal reached
      msg.linear.x = 0.0;
      msg.linear.y = 0.0;
      msg.linear.z = 0.0;
      RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 2000, "Goal reached!");
    }
    else if (distance < 10.0)
    {
      // Slow down when close to goal
      msg.linear.x = (error_x)/10.0 * constant_speed_;
      msg.linear.y = (error_y)/10.0 * constant_speed_;
      msg.linear.z = (error_z)/10.0 * constant_speed_;
      RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 2000, 
                          "Approaching goal, distance: %.2f", distance);
    }
    else
    {
      // Move at constant speed toward goal
      msg.linear.x = (error_x / distance) * constant_speed_;
      msg.linear.y = (error_y / distance) * constant_speed_;
      msg.linear.z = (error_z / distance) * constant_speed_;
      RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 2000, 
                          "Moving to goal, distance: %.2f", distance);
    }

    publisher_->publish(msg);
  }

  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr agent_odometry_subscriber_;
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr goal_subscriber_;
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