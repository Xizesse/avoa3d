#include <chrono>
#include <cmath>
#include <string>
#include <map>
#include <regex>
#include "rclcpp/rclcpp.hpp"
#include "custom_msgs/msg/element_characteristics_stamped.hpp"
#include "custom_msgs/msg/element_characteristics_array.hpp"
#include "nav_msgs/msg/odometry.hpp"

#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <geometry_msgs/msg/point_stamped.hpp>

using namespace std::chrono_literals;

class ObstaclePublisher : public rclcpp::Node
{
public:
  ObstaclePublisher()
  : Node("obstacle_publisher"), start_time_(this->get_clock()->now())
  {
    // Declare and get frame ID parameters with defaults
    this->declare_parameter<std::string>("fixed_frame", "map");
    this->declare_parameter<std::string>("agent_frame", "agent");

    this->declare_parameter<std::string>("topics.element_tracking", "/element_tracking/elements");
    
    fixed_frame_ = this->get_parameter("fixed_frame").as_string();
    agent_frame_ = this->get_parameter("agent_frame").as_string();

    std::string element_tracking_topic = this->get_parameter("topics.element_tracking").as_string();
    
    std::cout << "================================================================" << std::endl;
    std::cout << "============== OBSTACLE PUBLISHER INITIALIZATION ==============" << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << "Frame Settings:" << std::endl;
    std::cout << "  - Fixed Frame: " << fixed_frame_ << std::endl;
    std::cout << "  - Agent Frame: " << agent_frame_ << std::endl;
    std::cout << "Using topics:" << std::endl;
    std::cout << "  - Element Tracking: " << element_tracking_topic << std::endl;
    std::cout << "================================================================\n" << std::endl;

    tf_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    // Publisher for the array of obstacles
    publisher_ = this->create_publisher<custom_msgs::msg::ElementCharacteristicsArray>(element_tracking_topic, 10);
    
    // Discover obstacle topics
    discover_obstacle_topics();
    
    // Create a timer to discover new topics periodically
    discovery_timer_ = this->create_wall_timer(
      5000ms, std::bind(&ObstaclePublisher::discover_obstacle_topics, this));
    
    // Timer that calls timer_callback every 100 milliseconds
    timer_ = this->create_wall_timer(
      100ms, std::bind(&ObstaclePublisher::timer_callback, this));
      
    RCLCPP_INFO(this->get_logger(), "Waiting for obstacle data...");
  }

private:
  std::string fixed_frame_;
  std::string agent_frame_;
  
  // Track multiple obstacles, their data and status
  std::map<std::string, nav_msgs::msg::Odometry> obstacles_data_;
  std::map<std::string, bool> obstacles_active_;
  std::map<std::string, rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr> obstacle_subs_;
  
  // Regular expression to match obstacle odometry topics
  std::regex obstacle_topic_pattern_{"/model/obstacle.*?/odometry"};
  
  void discover_obstacle_topics()
  {
    // Get all available topics
    auto topic_names_and_types = this->get_topic_names_and_types();
    int new_topics = 0;
    
    for (const auto& [topic_name, types] : topic_names_and_types) {
      // Check if this is a potential obstacle topic
      if (std::regex_match(topic_name, obstacle_topic_pattern_)) {
        // Check if we already have this topic
        if (obstacle_subs_.find(topic_name) == obstacle_subs_.end()) {
          // Create a new subscription
          auto sub = this->create_subscription<nav_msgs::msg::Odometry>(
            topic_name, 10,
            [this, topic_name](const nav_msgs::msg::Odometry::SharedPtr msg) {
              this->odometry_callback(topic_name, msg);
            });
            
          obstacle_subs_[topic_name] = sub;
          obstacles_active_[topic_name] = false;
          new_topics++;
          
          RCLCPP_INFO(this->get_logger(), "Added subscription to obstacle topic: %s", topic_name.c_str());
        }
      }
    }
    
    if (new_topics > 0) {
      RCLCPP_INFO(this->get_logger(), "Discovered %d new obstacle topics, total: %ld", 
                  new_topics, obstacle_subs_.size());
    }
  }

  void odometry_callback(const std::string& topic, const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    obstacles_data_[topic] = *msg;
    obstacles_active_[topic] = true;
  }

  void timer_callback()
  {
    // Check if we have any active obstacles
    bool any_active = false;
    for (const auto& [topic, active] : obstacles_active_) {
      if (active) {
        any_active = true;
        break;
      }
    }
    
    if (!any_active) {
      RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 2000, 
                          "Waiting for obstacle data...");
      return;
    }
    
    // Create an array message to hold the elements
    auto array_message = custom_msgs::msg::ElementCharacteristicsArray();
    auto current_time = this->get_clock()->now();
    
    try {
      // Get the transform from world to agent frame
      geometry_msgs::msg::TransformStamped transform;
      transform = tf_buffer_->lookupTransform(
          agent_frame_,                   // target frame
          fixed_frame_,                   // source frame
          current_time,                   // time
          50ms);                          // timeout of 50ms 
      
      // Process each active obstacle
      int id = 1;
      for (const auto& [topic, odom] : obstacles_data_) {
        if (!obstacles_active_[topic]) continue;
        
        // Create a single element message
        auto element = custom_msgs::msg::ElementCharacteristicsStamped();
        
        // Create a PointStamped for the obstacle position in world frame
        geometry_msgs::msg::PointStamped obstacle_world;
        obstacle_world.header.frame_id = fixed_frame_;
        obstacle_world.header.stamp = current_time;
        obstacle_world.point.x = odom.pose.pose.position.x;
        obstacle_world.point.y = odom.pose.pose.position.y;
        obstacle_world.point.z = odom.pose.pose.position.z;

        geometry_msgs::msg::PointStamped obstacle_agent_frame;
        
        // Apply the transform to the obstacle point
        tf2::doTransform(obstacle_world, obstacle_agent_frame, transform);
        
        // Now obstacle_agent_frame.point has the coordinates in the agent's frame
        element.header.stamp = current_time;
        element.header.frame_id = agent_frame_;
        element.id = id++;  // Assign unique ID to each obstacle
        element.type = 1;
        element.dynamic = false;
        
        // Use the transformed coordinates
        element.pose.position.x = obstacle_agent_frame.point.x;
        element.pose.position.y = obstacle_agent_frame.point.y;
        element.pose.position.z = obstacle_agent_frame.point.z;
        
        // Transform the orientation
        geometry_msgs::msg::QuaternionStamped orientation_world;
        orientation_world.header.frame_id = fixed_frame_;
        orientation_world.header.stamp = current_time;
        orientation_world.quaternion = odom.pose.pose.orientation;
        
        geometry_msgs::msg::QuaternionStamped orientation_agent;
        try {
          tf2::doTransform(orientation_world, orientation_agent, transform);
          element.pose.orientation = orientation_agent.quaternion;
        } catch (const tf2::TransformException & ex) {
          RCLCPP_WARN(this->get_logger(), "Orientation transform failed: %s", ex.what());
          // Fallback to original orientation
          element.pose.orientation = odom.pose.pose.orientation;
        }
        
        // Transform the velocity vector
        geometry_msgs::msg::Vector3Stamped vel_world;
        vel_world.header.frame_id = fixed_frame_;
        vel_world.header.stamp = current_time;
        vel_world.vector = odom.twist.twist.linear;
        
        geometry_msgs::msg::Vector3Stamped vel_agent;
        try {
          tf2::doTransform(vel_world, vel_agent, transform);
          // Set the obstacle's velocity in the agent's frame
          element.velocity.x = vel_agent.vector.x;
          element.velocity.y = vel_agent.vector.y;
          element.velocity.z = vel_agent.vector.z;
        } catch (const tf2::TransformException & ex) {
          RCLCPP_WARN(this->get_logger(), "Velocity transform failed: %s", ex.what());
          // Fallback to zeros
          element.velocity.x = 0.0;
          element.velocity.y = 0.0;
          element.velocity.z = 0.0;
        }
        
        // Set the size and protective zone
        element.size.x = 1.0;
        element.size.y = 1.0;
        element.size.z = 1.0;
        element.protective_zone = 0.0;
        
        array_message.elements.push_back(element);
      }
      
      // Publish the array with all obstacles
      publisher_->publish(array_message);
      RCLCPP_DEBUG(this->get_logger(), "Published %zu obstacles", array_message.elements.size());
      
    } catch (const tf2::TransformException &ex) {
      RCLCPP_ERROR(this->get_logger(), "Failed to transform obstacles to agent frame: %s", ex.what());
      
      // Fallback: publish untransformed data
      int id = 1;
      for (const auto& [topic, odom] : obstacles_data_) {
        if (!obstacles_active_[topic]) continue;
        
        auto element = custom_msgs::msg::ElementCharacteristicsStamped();
        element.header.stamp = current_time;
        element.header.frame_id = fixed_frame_;
        element.id = id++;
        element.type = 1;
        element.dynamic = false;
        
        // Use original coordinates
        element.pose.position.x = odom.pose.pose.position.x;
        element.pose.position.y = odom.pose.pose.position.y;
        element.pose.position.z = odom.pose.pose.position.z;
        element.pose.orientation = odom.pose.pose.orientation;
        
        // Use original velocity
        element.velocity.x = odom.twist.twist.linear.x;
        element.velocity.y = odom.twist.twist.linear.y;
        element.velocity.z = odom.twist.twist.linear.z;
        
        // Set the size and protective zone
        element.size.x = 0.5;
        element.size.y = 0.5;
        element.size.z = 0.5;
        element.protective_zone = 0.0;
        
        array_message.elements.push_back(element);
      }
      
      RCLCPP_WARN(this->get_logger(), "Using untransformed coordinates as fallback");
      
      if (!array_message.elements.empty()) {
        publisher_->publish(array_message);
      }
    }
  }

  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;

  rclcpp::Publisher<custom_msgs::msg::ElementCharacteristicsArray>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::TimerBase::SharedPtr discovery_timer_;
  rclcpp::Time start_time_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ObstaclePublisher>());
  rclcpp::shutdown();
  return 0;
}