#include <chrono>
#include <memory>
#include <string>
#include <cmath>  // For sqrt, max, etc.

#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/marker.hpp"
#include "visualization_msgs/msg/marker_array.hpp"  // Added for MarkerArray
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

#include "custom_msgs/msg/element_characteristics_stamped.hpp"
#include "custom_msgs/msg/element_characteristics_array.hpp"

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
    // Declare and get frame ID parameters with defaults
    this->declare_parameter<std::string>("fixed_frame", "map");
    this->declare_parameter<std::string>("agent_frame", "agent");
    
    // Declare subscription topic parameters
    this->declare_parameter<std::string>("topics.cmd_vel", "/model/agente/cmd_vel");
    this->declare_parameter<std::string>("topics.desired_vel", "/model/agente/desired_vel");
    this->declare_parameter<std::string>("topics.agent_odometry", "/model/agente/odometry");
    this->declare_parameter<std::string>("topics.obstacle_odometry", "/model/obstacle/odometry");
    this->declare_parameter<std::string>("topics.element_tracking", "/element_tracking/elements");
      
    fixed_frame = this->get_parameter("fixed_frame").as_string();
    agent_frame = this->get_parameter("agent_frame").as_string();

    const std::string agent_marker_topic = "/markers/agent_marker";
    const std::string obstacle_array_topic = "/markers/obstacle_array";  // Changed to array
    const std::string desired_vel_marker_topic = "/markers/desired_vel";
    const std::string cmd_vel_marker_topic = "/markers/cmd_vel";
    const std::string visualization_marker_topic = "/visualization_marker";
    
    // Get subscription topic parameters
    std::string cmd_vel_topic = this->get_parameter("topics.cmd_vel").as_string();
    std::string desired_vel_topic = this->get_parameter("topics.desired_vel").as_string();
    std::string agent_odometry_topic = this->get_parameter("topics.agent_odometry").as_string();
    std::string obstacle_odometry_topic = this->get_parameter("topics.obstacle_odometry").as_string();
    std::string element_tracking_topic = this->get_parameter("topics.element_tracking").as_string();
    
    // RVIZ MARKER NODE
    std::cout << "================================================================" << std::endl;
    std::cout << "=================== RVIZ MARKER INITIALIZATION =================" << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << "Frame Settings:" << std::endl;
    std::cout << "  - Fixed Frame: " << fixed_frame << std::endl;
    std::cout << "  - Agent Frame: " << agent_frame << std::endl;
    std::cout << "Using topics:" << std::endl;
    std::cout << "  - cmd_vel: " << cmd_vel_topic << std::endl;
    std::cout << "  - desired_vel: " << desired_vel_topic << std::endl;
    std::cout << "  - agent_odometry: " << agent_odometry_topic << std::endl;
    std::cout << "  - obstacle_odometry: " << obstacle_odometry_topic << std::endl;
    std::cout << "  - element_tracking: " << element_tracking_topic << std::endl;

    std::cout << "================================================================\n" << std::endl;

    
    visual_tools_ = std::make_shared<rviz_visual_tools::RvizVisualTools>(agent_frame, "/visualization_marker", this); 
    visual_tools_->loadMarkerPub();
    visual_tools_->enableBatchPublishing();
      
    //! PUBLISHERS
    // Publishers for Markers
    agent_publisher_ = this->create_publisher<visualization_msgs::msg::Marker>(agent_marker_topic, 10);
    // New MarkerArray publisher for obstacles
    obstacle_array_publisher_ = this->create_publisher<visualization_msgs::msg::MarkerArray>(obstacle_array_topic, 10);
    // Publishers for TwistStamped
    velocity_desired_publisher_ = this->create_publisher<geometry_msgs::msg::TwistStamped>(desired_vel_marker_topic, 10);
    velocity_cmd_publisher_ = this->create_publisher<geometry_msgs::msg::TwistStamped>(cmd_vel_marker_topic, 10);
    
    //! SUBSCRIBERS
    velocity_cmd_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
      cmd_vel_topic, 10, std::bind(&MarkerPublisher::velocity_cmd_callback, this, std::placeholders::_1));
    velocity_desired_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
      desired_vel_topic, 10, std::bind(&MarkerPublisher::velocity_desired_callback, this, std::placeholders::_1));
    agent_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      agent_odometry_topic, 10, std::bind(&MarkerPublisher::agent_odometry_callback, this, std::placeholders::_1));
    obstacle_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      obstacle_odometry_topic, 10, std::bind(&MarkerPublisher::obstacle_odometry_callback, this, std::placeholders::_1)); 
    element_tracking_subscriber_ = this->create_subscription<custom_msgs::msg::ElementCharacteristicsArray>(
      element_tracking_topic, 10, std::bind(&MarkerPublisher::element_tracking_callback, this, std::placeholders::_1));
    
    timer_ = this->create_wall_timer(100ms, std::bind(&MarkerPublisher::timer_callback, this));
  }

private:
  void publish_agent()
  {
    //! AGENT MARKER
    auto agent_marker_msg = visualization_msgs::msg::Marker();
    agent_marker_msg.header.frame_id = agent_frame; 
    agent_marker_msg.header.stamp = this->now();
    agent_marker_msg.ns = "basic_shapes";
    agent_marker_msg.id = 0;
    agent_marker_msg.type = visualization_msgs::msg::Marker::CUBE;
    agent_marker_msg.action = visualization_msgs::msg::Marker::ADD;
    agent_marker_msg.pose.position.x = 0.0;
    agent_marker_msg.pose.position.y = 0.0;
    agent_marker_msg.pose.position.z = 0.0;
    agent_marker_msg.pose.orientation.w = 1.0;
    agent_marker_msg.pose.orientation.x = 0.0;
    agent_marker_msg.pose.orientation.y = 0.0;
    agent_marker_msg.pose.orientation.z = 0.0;
    agent_marker_msg.scale.x = 1.0;
    agent_marker_msg.scale.y = 1.0;
    agent_marker_msg.scale.z = 0.1;
    agent_marker_msg.color.r = 1.0f;
    agent_marker_msg.color.g = 0.0f;
    agent_marker_msg.color.b = 1.0f;
    agent_marker_msg.color.a = 0.3;
    agent_marker_msg.lifetime = rclcpp::Duration(1, 0);  // 1 second lifetime

    agent_publisher_->publish(agent_marker_msg);
  }  
  
  void publish_obstacle()
  {
    // Create a marker array to hold all obstacles
    auto marker_array = visualization_msgs::msg::MarkerArray();
    
    // First add a DELETE_ALL marker to clear previous markers
    auto delete_marker = visualization_msgs::msg::Marker();
    delete_marker.header.frame_id = agent_frame;
    delete_marker.header.stamp = this->now();
    delete_marker.ns = "obstacles";
    delete_marker.id = 0;
    delete_marker.action = visualization_msgs::msg::Marker::DELETEALL;
    marker_array.markers.push_back(delete_marker);
    
    // Add a DELETE_ALL marker for velocity markers too
    auto delete_vel_marker = visualization_msgs::msg::Marker();
    delete_vel_marker.header.frame_id = agent_frame;
    delete_vel_marker.header.stamp = this->now();
    delete_vel_marker.ns = "obstacle_velocities";
    delete_vel_marker.id = 0;
    delete_vel_marker.action = visualization_msgs::msg::Marker::DELETEALL;
    marker_array.markers.push_back(delete_vel_marker);
    
    // Log the number of obstacles (but not too frequently)
    RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 2000, 
                        "Publishing %zu obstacles", obstacles_.elements.size());
    
    // Start IDs at 1 since 0 is used for DELETE_ALL
    int id = 1;
    
    // Process each obstacle
    for (auto& obstacle : obstacles_.elements) {
      // Create marker for this obstacle
      auto obstacle_marker = visualization_msgs::msg::Marker();
      obstacle_marker.header.frame_id = agent_frame;
      obstacle_marker.header.stamp = this->now();
      obstacle_marker.ns = "obstacles";
      obstacle_marker.id = id++;  // Use incremental ID
      obstacle_marker.type = visualization_msgs::msg::Marker::SPHERE;
      obstacle_marker.action = visualization_msgs::msg::Marker::ADD;
      
      // Position and orientation
      obstacle_marker.pose.position.x = obstacle.pose.position.x;
      obstacle_marker.pose.position.y = obstacle.pose.position.y;
      obstacle_marker.pose.position.z = obstacle.pose.position.z;
      obstacle_marker.pose.orientation = obstacle.pose.orientation;
      
      // Size (use at least 0.5m for visibility)
      obstacle_marker.scale.x = std::max(obstacle.size.x, 0.5);
      obstacle_marker.scale.y = std::max(obstacle.size.y, 0.5);
      obstacle_marker.scale.z = std::max(obstacle.size.z, 0.5);
      
      // Green color
      obstacle_marker.color.r = 0.0f;
      obstacle_marker.color.g = 1.0f;
      obstacle_marker.color.b = 0.0f;
      obstacle_marker.color.a = 0.9;
      
      // Use a longer lifetime (1 second)
      obstacle_marker.lifetime = rclcpp::Duration(1, 0);
      
      // Add to the array
      marker_array.markers.push_back(obstacle_marker);
      
      // Add velocity vector visualization as an arrow
      // Calculate velocity magnitude
      float speed = std::sqrt(
        obstacle.velocity.x * obstacle.velocity.x +
        obstacle.velocity.y * obstacle.velocity.y +
        obstacle.velocity.z * obstacle.velocity.z
      );
      
      // Only show velocity for moving obstacles (speed > 0.1 m/s)
      if (speed > 0.1) {
        auto velocity_marker = visualization_msgs::msg::Marker();
        velocity_marker.header.frame_id = agent_frame;
        velocity_marker.header.stamp = this->now();
        velocity_marker.ns = "obstacle_velocities";
        velocity_marker.id = id++;  // Use next ID
        velocity_marker.type = visualization_msgs::msg::Marker::ARROW;
        velocity_marker.action = visualization_msgs::msg::Marker::ADD;
        
        // Start at obstacle position
        velocity_marker.pose.position = obstacle.pose.position;
        
        // Calculate orientation from velocity vector
        if (speed > 0) {
          // Create normalized velocity vector
          tf2::Vector3 vel_vector(
            obstacle.velocity.x / speed,
            obstacle.velocity.y / speed, 
            obstacle.velocity.z / speed
          );
          
          // Default reference direction (arrow points along x-axis)
          tf2::Vector3 reference(1, 0, 0);
          
          // Calculate rotation axis and angle
          tf2::Vector3 axis = reference.cross(vel_vector);
          float angle = 0.0;
          
          // Set quaternion based on axis and angle
          tf2::Quaternion quat;
          
          if (axis.length() > 0.001) {
            // Normal case - calculate angle and rotate
            angle = std::acos(reference.dot(vel_vector));
            quat.setRotation(axis.normalize(), angle);
          } else if (vel_vector.x() < 0) {
            // Special case - velocity pointing backward along x-axis
            quat.setRotation(tf2::Vector3(0, 1, 0), M_PI);
          } else {
            // Default - pointing forward along x-axis
            quat.setRotation(tf2::Vector3(0, 1, 0), 0);
          }
          
          // Convert to geometry_msgs quaternion
          velocity_marker.pose.orientation.x = quat.x();
          velocity_marker.pose.orientation.y = quat.y();
          velocity_marker.pose.orientation.z = quat.z();
          velocity_marker.pose.orientation.w = quat.w();
        } else {
          // Default identity orientation if no velocity
          velocity_marker.pose.orientation.w = 1.0;
        }
        
        // Scale arrow by velocity - length proportional to speed
        float arrow_length = std::max(speed, 0.5f);  // At least 0.5m for visibility
        velocity_marker.scale.x = arrow_length;  // Length along x-axis
        velocity_marker.scale.y = 0.1;           // Width
        velocity_marker.scale.z = 0.1;           // Height
        
        // Blue color for velocity
        velocity_marker.color.r = 0.0f;
        velocity_marker.color.g = 0.0f;
        velocity_marker.color.b = 1.0f;
        velocity_marker.color.a = 0.8;
        
        velocity_marker.lifetime = rclcpp::Duration(1, 0);
        
        // Add velocity marker to array
        marker_array.markers.push_back(velocity_marker);
      }
    }
    
    // Publish the entire array at once if we have any markers
    if (!marker_array.markers.empty()) {
      obstacle_array_publisher_->publish(marker_array);
    }
  }

  void publish_velocity_cmd()
  {
    // Convert Twist to TwistStamped for command velocity
    auto twist_stamped_msg = geometry_msgs::msg::TwistStamped();
    twist_stamped_msg.header.stamp = this->now();
    twist_stamped_msg.header.frame_id = agent_frame;  // Use parameter instead of hardcoded "agent"
    twist_stamped_msg.twist = velocity_cmd_;
    
    velocity_cmd_publisher_->publish(twist_stamped_msg);
  }
  
  void publish_velocity_desired()
  {
    // Convert Twist to TwistStamped for desired velocity
    auto twist_stamped_msg = geometry_msgs::msg::TwistStamped();
    twist_stamped_msg.header.stamp = this->now();
    twist_stamped_msg.header.frame_id = agent_frame;  // Use parameter instead of hardcoded "agent"
    twist_stamped_msg.twist = velocity_desired_;
    
    velocity_desired_publisher_->publish(twist_stamped_msg);
  }
  
  //! Callbacks
  void timer_callback()
  {
    publish_agent();
    publish_obstacle();
    publish_velocity_cmd();
    publish_velocity_desired();
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
  } 

  void obstacle_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    obstacle_odometry_ = *msg;
  }

  void element_tracking_callback(const custom_msgs::msg::ElementCharacteristicsArray::SharedPtr msg)
  {
    obstacles_ = *msg;
  }

  rclcpp::TimerBase::SharedPtr timer_;
  std::string fixed_frame;
  std::string agent_frame;
  rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr agent_publisher_;
  // Changed from single marker publisher to marker array publisher
  rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr obstacle_array_publisher_;
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr velocity_desired_publisher_;
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr velocity_cmd_publisher_;
  
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr velocity_cmd_subscriber_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr velocity_desired_subscriber_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr agent_odometry_subscriber_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr obstacle_odometry_subscriber_;
  rclcpp::Subscription<custom_msgs::msg::ElementCharacteristicsArray>::SharedPtr element_tracking_subscriber_;
  
  geometry_msgs::msg::Twist velocity_cmd_;
  geometry_msgs::msg::Twist velocity_desired_;
  nav_msgs::msg::Odometry agent_odometry_;
  nav_msgs::msg::Odometry obstacle_odometry_;
  custom_msgs::msg::ElementCharacteristicsArray obstacles_;
  size_t count_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MarkerPublisher>());
  rclcpp::shutdown();
  return 0;
}