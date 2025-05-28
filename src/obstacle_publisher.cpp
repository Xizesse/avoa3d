#include <chrono>
#include <string>
#include <vector>
#include "rclcpp/rclcpp.hpp"
#include "custom_msgs/msg/element_characteristics_stamped.hpp"
#include "custom_msgs/msg/element_characteristics_array.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

using namespace std::chrono_literals;

class SimpleObstaclePublisher : public rclcpp::Node
{
public:
  SimpleObstaclePublisher()
  : Node("obstacle_publisher")
  {
    // Declare parameters
    this->declare_parameter<std::string>("fixed_frame", "map");
    this->declare_parameter<std::string>("agent_frame", "agent");
    this->declare_parameter<int>("num_obstacles", 11);
    
    fixed_frame_ = this->get_parameter("fixed_frame").as_string();
    agent_frame_ = this->get_parameter("agent_frame").as_string();
    int num_obstacles = this->get_parameter("num_obstacles").as_int();
    
    RCLCPP_INFO(this->get_logger(), "Starting Simple Obstacle Publisher");
    RCLCPP_INFO(this->get_logger(), "Fixed frame: %s, Agent frame: %s", 
                fixed_frame_.c_str(), agent_frame_.c_str());
    RCLCPP_INFO(this->get_logger(), "Tracking %d obstacles", num_obstacles);
    
    // Set up TF2
    tf_buffer_ = std::make_shared<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
    
    // Create publisher
    publisher_ = this->create_publisher<custom_msgs::msg::ElementCharacteristicsArray>(
                 "/element_tracking/elements", 10);
    
    // Create subscriptions for the fixed number of obstacles
    obstacles_data_.resize(num_obstacles);
    obstacles_active_.resize(num_obstacles, false);
    
    for (int i = 0; i < num_obstacles; i++) {
      std::string topic = "/model/obstacle_" + std::to_string(i) + "/odometry";
      
      auto callback = [this, i](const nav_msgs::msg::Odometry::SharedPtr msg) {
        obstacles_data_[i] = *msg;
        obstacles_active_[i] = true;
      };
      
      obstacle_subs_.push_back(
        this->create_subscription<nav_msgs::msg::Odometry>(topic, 10, callback)
      );
      
      RCLCPP_INFO(this->get_logger(), "Subscribed to: %s", topic.c_str());
    }
    
    // Create timer for publishing
    timer_ = this->create_wall_timer(100ms, std::bind(&SimpleObstaclePublisher::publish_obstacles, this));
  }

private:
  void publish_obstacles()
  {
    // Check if we have any active obstacles
    bool any_active = false;
    for (bool active : obstacles_active_) {
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
    
    // Create element array message
    auto array_msg = custom_msgs::msg::ElementCharacteristicsArray();
    auto current_time = this->get_clock()->now();
    
    try {
      // Get transform from world to agent frame
      geometry_msgs::msg::TransformStamped transform = tf_buffer_->lookupTransform(
          agent_frame_,    // target frame
          fixed_frame_,    // source frame
          current_time,    // time
          50ms             // timeout
      );
      
      // Process each active obstacle
      for (size_t i = 0; i < obstacles_data_.size(); i++) {
        if (!obstacles_active_[i]) continue;
        
        // Create element message
        auto element = custom_msgs::msg::ElementCharacteristicsStamped();
        element.header.stamp = current_time;
        element.header.frame_id = agent_frame_;
        element.id = static_cast<int>(i + 1);  // IDs start at 1
        element.type = 1;
        element.dynamic = false;
        
        // Get obstacle pose in fixed frame
        geometry_msgs::msg::PointStamped obstacle_world;
        obstacle_world.header.frame_id = fixed_frame_;
        obstacle_world.header.stamp = current_time;
        obstacle_world.point.x = obstacles_data_[i].pose.pose.position.x;
        obstacle_world.point.y = obstacles_data_[i].pose.pose.position.y;
        obstacle_world.point.z = obstacles_data_[i].pose.pose.position.z;
        
        // Transform position to agent frame
        geometry_msgs::msg::PointStamped obstacle_agent;
        tf2::doTransform(obstacle_world, obstacle_agent, transform);
        
        // Set position
        element.pose.position.x = obstacle_agent.point.x;
        element.pose.position.y = obstacle_agent.point.y;
        element.pose.position.z = obstacle_agent.point.z;
        
        // Transform orientation
        geometry_msgs::msg::QuaternionStamped orientation_world;
        orientation_world.header = obstacle_world.header;
        orientation_world.quaternion = obstacles_data_[i].pose.pose.orientation;
        
        geometry_msgs::msg::QuaternionStamped orientation_agent;
        tf2::doTransform(orientation_world, orientation_agent, transform);
        element.pose.orientation = orientation_agent.quaternion;
        
        // Transform velocity to agent frame
        geometry_msgs::msg::Vector3Stamped velocity_world;
        velocity_world.header.frame_id = fixed_frame_;
        velocity_world.header.stamp = current_time;
        velocity_world.vector.x = obstacles_data_[i].twist.twist.linear.x;
        velocity_world.vector.y = obstacles_data_[i].twist.twist.linear.y;
        velocity_world.vector.z = obstacles_data_[i].twist.twist.linear.z;
        
        geometry_msgs::msg::Vector3Stamped velocity_agent;
        tf2::doTransform(velocity_world, velocity_agent, transform);
        
        // Set transformed velocity
        element.velocity.x = velocity_agent.vector.x;
        element.velocity.y = velocity_agent.vector.y;
        element.velocity.z = velocity_agent.vector.z;
        
        // Set size (fixed for all obstacles)
        element.size.x = 1.0;
        element.size.y = 1.0;
        element.size.z = 1.0;
        element.protective_zone = 0.0;
        
        // Add to array
        array_msg.elements.push_back(element);
      }
      
      // Publish array
      publisher_->publish(array_msg);
      RCLCPP_DEBUG(this->get_logger(), "Published %zu obstacles", array_msg.elements.size());
      
    } catch (const tf2::TransformException &ex) {
      RCLCPP_WARN(this->get_logger(), "Transform error: %s", ex.what());
    }
  }

  // Member variables
  std::string fixed_frame_;
  std::string agent_frame_;
  std::vector<nav_msgs::msg::Odometry> obstacles_data_;
  std::vector<bool> obstacles_active_;
  std::vector<rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr> obstacle_subs_;
  
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  rclcpp::Publisher<custom_msgs::msg::ElementCharacteristicsArray>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SimpleObstaclePublisher>());
  rclcpp::shutdown();
  return 0;
}