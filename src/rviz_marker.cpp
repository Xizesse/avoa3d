#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/marker.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Vector3.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include "geometry_msgs/msg/transform_stamped.hpp"
#include <rviz_visual_tools/rviz_visual_tools.hpp>
#include <Eigen/Geometry>

  #include <tf2/LinearMath/Quaternion.h>
  #include <tf2/LinearMath/Vector3.h>


//######################################
// Node to publish markers for RVIZ 
//######################################

using namespace std::chrono_literals;

class MarkerPublisher : public rclcpp::Node
{
public:

  std::shared_ptr<rviz_visual_tools::RvizVisualTools> visual_tools_;
  MarkerPublisher()
  : Node("marker_publisher"), count_(0)
  {
    tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);
    visual_tools_ = std::make_shared<rviz_visual_tools::RvizVisualTools>("map", "/visualization_marker", this); 
    visual_tools_->loadMarkerPub();
    visual_tools_->enableBatchPublishing();
    
    //! PUBLISHERS
    // Publishers for Markers
    agent_publisher_ = this->create_publisher<visualization_msgs::msg::Marker>("/markers/agent_marker", 10);
    obstacle_publisher_ = this->create_publisher<visualization_msgs::msg::Marker>("/markers/obstacle_marker", 10);
    goal_publisher_ = this->create_publisher<visualization_msgs::msg::Marker>("/markers/goal_marker", 10);
    // Publishers for TwistStamped
    velocity_desired_publisher_ = this->create_publisher<geometry_msgs::msg::TwistStamped>("/markers/desired_vel", 10);
    velocity_cmd_publisher_ = this->create_publisher<geometry_msgs::msg::TwistStamped>("/markers/cmd_vel", 10);
    
    //! SUBSCRIBERS
    velocity_cmd_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
      "/model/agente/cmd_vel", 10, std::bind(&MarkerPublisher::velocity_cmd_callback, this, std::placeholders::_1));
    velocity_desired_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
      "/model/agente/desired_vel", 10, std::bind(&MarkerPublisher::velocity_desired_callback, this, std::placeholders::_1));
    agent_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/model/agente/odometry", 10, std::bind(&MarkerPublisher::agent_odometry_callback, this, std::placeholders::_1));
    obstacle_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/model/obstacle/odometry", 10, std::bind(&MarkerPublisher::obstacle_odometry_callback, this, std::placeholders::_1)); 
    goal_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/model/goal/odometry", 10, std::bind(&MarkerPublisher::goal_odometry_callback, this, std::placeholders::_1));
    
    timer_ = this->create_wall_timer(100ms, std::bind(&MarkerPublisher::timer_callback, this));
  }

private:
  void publish_agent()
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
    agent_marker_msg.lifetime = rclcpp::Duration(std::chrono::nanoseconds(static_cast<int64_t>(1))); 

    agent_publisher_->publish(agent_marker_msg);
  }  
  
  void publish_obstacle()
  {
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
    obstacle_marker_msg.lifetime = rclcpp::Duration(std::chrono::nanoseconds(static_cast<int64_t>(1))); 

    obstacle_publisher_->publish(obstacle_marker_msg);
  }

  void publish_goal()
  {
    //! GOAL MARKER
    auto goal_marker_msg = visualization_msgs::msg::Marker();
    goal_marker_msg.header.frame_id = "map";
    goal_marker_msg.header.stamp = this->now();
    goal_marker_msg.ns = "basic_shapes";
    goal_marker_msg.id = 0;
    goal_marker_msg.type = visualization_msgs::msg::Marker::CUBE;
    goal_marker_msg.action = visualization_msgs::msg::Marker::ADD;
    goal_marker_msg.pose.position.x = goal_odometry_.pose.pose.position.x;
    goal_marker_msg.pose.position.y = goal_odometry_.pose.pose.position.y;
    goal_marker_msg.pose.position.z = goal_odometry_.pose.pose.position.z;
    goal_marker_msg.pose.orientation = goal_odometry_.pose.pose.orientation;
  
    goal_marker_msg.scale.x = 1.0;
    goal_marker_msg.scale.y = 1.0;
    goal_marker_msg.scale.z = 1.0;
    goal_marker_msg.color.r = 1.0f;
    goal_marker_msg.color.g = 0.0f;
    goal_marker_msg.color.b = 1.0f;
    goal_marker_msg.color.a = 0.2;
    goal_marker_msg.lifetime = rclcpp::Duration(std::chrono::nanoseconds(static_cast<int64_t>(1))); 

    goal_publisher_->publish(goal_marker_msg);
  }  
  
  void publish_velocity_cmd()
  {
    // Convert Twist to TwistStamped for command velocity
    auto twist_stamped_msg = geometry_msgs::msg::TwistStamped();
    twist_stamped_msg.header.stamp = this->now();
    twist_stamped_msg.header.frame_id = "agent"; // Use agent frame for velocity
    twist_stamped_msg.twist = velocity_cmd_;
    
    velocity_cmd_publisher_->publish(twist_stamped_msg);
  }
  
  void publish_velocity_desired()
  {
    // Convert Twist to TwistStamped for desired velocity
    auto twist_stamped_msg = geometry_msgs::msg::TwistStamped();
    twist_stamped_msg.header.stamp = this->now();
    twist_stamped_msg.header.frame_id = "agent"; // Use agent frame for velocity
    twist_stamped_msg.twist = velocity_desired_;
    
    velocity_desired_publisher_->publish(twist_stamped_msg);
  }
  
  void publishVelocityObstacleCone()
  {
      visual_tools_->deleteAllMarkers();
      
      // Get positions
      Eigen::Vector3d agent_position(
          agent_odometry_.pose.pose.position.x,
          agent_odometry_.pose.pose.position.y,
          agent_odometry_.pose.pose.position.z
      );
      
      Eigen::Vector3d obstacle_position(
          obstacle_odometry_.pose.pose.position.x,
          obstacle_odometry_.pose.pose.position.y,
          obstacle_odometry_.pose.pose.position.z
      );
      
      // Agent -> Obstacle 
      Eigen::Vector3d direction = obstacle_position - agent_position;
      double distance = direction.norm();
      
      // Skip if the obstacle is too close or too far
      if (distance < 0.1 || distance > 100.0) {
          visual_tools_->trigger();
          return;
      }
      
      direction.normalize();
      
      double obstacle_radius = 0.5; 
      
      double cone_angle =  std::atan(distance / obstacle_radius) * distance ;

      
      geometry_msgs::msg::Pose cone_pose;
      cone_pose.position.x = agent_position.x();
      cone_pose.position.y = agent_position.y();
      cone_pose.position.z = agent_position.z();
  
      // Rotate the default X-axis (1,0,0) to align with the direction vector
      tf2::Vector3 x_axis(1.0, 0.0, 0.0); // Original X-axis
      tf2::Vector3 target_dir(direction.x(), direction.y(), direction.z()); // Desired direction
  
      // Compute rotation axis (cross product)
      tf2::Vector3 rotation_axis = x_axis.cross(target_dir);
      double rotation_angle = std::acos(x_axis.dot(target_dir)); // Angle between vectors
  
      tf2::Quaternion quat;
      if (rotation_axis.length2() > 1e-6) { // Avoid division by zero
          rotation_axis.normalize();
          quat.setRotation(rotation_axis, rotation_angle);
      } else {
          quat.setValue(0, 0, 0, 1); // No rotation needed
      }
  
      cone_pose.orientation.x = quat.x();
      cone_pose.orientation.y = quat.y();
      cone_pose.orientation.z = quat.z();
      cone_pose.orientation.w = quat.w();
      
      // The cone length should be the distance to the obstacle
      
      // Publish the cone
      visual_tools_->publishCone(cone_pose, cone_angle, rviz_visual_tools::TRANSLUCENT, distance);
      
      // Publish a line from agent to obstacle for reference
      visual_tools_->publishLine(agent_position, obstacle_position, rviz_visual_tools::RED, rviz_visual_tools::LARGE);
      
      // Trigger batch publishing
      visual_tools_->trigger();
      
      //RCLCPP_INFO(this->get_logger(), "Publishing velocity obstacle cone with distance %.2f", distance);
  }
  
  //! Callbacks

  void timer_callback()
  {
    publish_agent();
    publish_obstacle();
    publish_goal();
    publish_velocity_cmd();
    publish_velocity_desired();
    publishVelocityObstacleCone();
    
  }

  void velocity_cmd_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
  {
    velocity_cmd_ = *msg;
  }

  void velocity_desired_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
  {
    velocity_desired_ = *msg;
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

  void goal_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    goal_odometry_ = *msg;
  }

  void obstacle_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    obstacle_odometry_ = *msg;
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr agent_publisher_;
  rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr goal_publisher_;
  rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr obstacle_publisher_;
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr velocity_desired_publisher_;
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr velocity_cmd_publisher_;
  
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr velocity_cmd_subscriber_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr velocity_desired_subscriber_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr agent_odometry_subscriber_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr obstacle_odometry_subscriber_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr goal_odometry_subscriber_;
  
  geometry_msgs::msg::Twist velocity_cmd_;
  geometry_msgs::msg::Twist velocity_desired_;
  nav_msgs::msg::Odometry agent_odometry_;
  nav_msgs::msg::Odometry obstacle_odometry_;
  nav_msgs::msg::Odometry goal_odometry_;
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