#include <vector>
#include <string>
#include "rclcpp/rclcpp.hpp"
#include "custom_msgs/msg/element_characteristics_stamped.hpp"
#include "custom_msgs/msg/element_characteristics_array.hpp"
#include "obstacle_detector/msg/obstacles.hpp"
#include "obstacle_detector/msg/circle_obstacle.hpp"

using namespace std::chrono_literals;

class EnhancedObstacleConverterNode : public rclcpp::Node
{
public:
  EnhancedObstacleConverterNode()
  : Node("enhanced_obstacle_converter")
  {
    // Declare parameters
    this->declare_parameter<std::string>("frame_id", "lidar");
    this->declare_parameter<double>("default_height", 0.5);
    this->declare_parameter<double>("min_velocity_threshold", 0.05);
    this->declare_parameter<double>("default_pz", 0.0);
    
    frame_id_ = this->get_parameter("frame_id").as_string();
    default_height_ = this->get_parameter("default_height").as_double();
    min_velocity_threshold_ = this->get_parameter("min_velocity_threshold").as_double();
    default_pz_ = this->get_parameter("default_pz").as_double();
    
    RCLCPP_INFO(this->get_logger(), "Starting Enhanced Obstacle Converter Node");
    RCLCPP_INFO(this->get_logger(), "Using frame: %s", frame_id_.c_str());
    
    // Create publisher for element tracking message
    elements_publisher_ = this->create_publisher<custom_msgs::msg::ElementCharacteristicsArray>(
                         "/element_tracking/elements", 10);
    
    // Create subscription to obstacle detector messages
    obstacles_subscription_ = this->create_subscription<obstacle_detector::msg::Obstacles>(
      "/obstacles", 10,
      std::bind(&EnhancedObstacleConverterNode::obstacles_callback, this, std::placeholders::_1)
    );
    
    RCLCPP_INFO(this->get_logger(), "Subscribed to: /obstacles");
    RCLCPP_INFO(this->get_logger(), "Publishing to: /element_tracking/elements");
  }

private:
  void obstacles_callback(const obstacle_detector::msg::Obstacles::SharedPtr msg)
  {
    auto current_time = this->get_clock()->now();
    
    // Debug output - report received message
    // RCLCPP_INFO(this->get_logger(), "Received obstacles message with %zu circles and %zu segments", 
    //            msg->circles.size(), msg->segments.size());
    
    // Create element array message
    auto elements_msg = custom_msgs::msg::ElementCharacteristicsArray();
    
    // Process circle obstacles
    for (size_t i = 0; i < msg->circles.size(); i++) {
      const auto& circle = msg->circles[i];
      
      // Detect if the object is moving based on velocity
      bool is_moving = (std::abs(circle.velocity.x) > min_velocity_threshold_ || 
                       std::abs(circle.velocity.y) > min_velocity_threshold_ ||
                       std::abs(circle.velocity.z) > min_velocity_threshold_);
      
      // Debug each circle
      // RCLCPP_INFO(this->get_logger(), "Circle %zu: Position (%.2f, %.2f, %.2f), Radius: %.2f, Velocity (%.2f, %.2f, %.2f), Moving: %s", 
      //            i, circle.center.x, circle.center.y, circle.center.z, circle.radius,
      //            circle.velocity.x, circle.velocity.y, circle.velocity.z,
      //            is_moving ? "true" : "false");
      
      // Create element message with enhanced fields
      auto element = custom_msgs::msg::ElementCharacteristicsStamped();
      element.header.stamp = current_time;
      element.header.frame_id = frame_id_;  // Use specified frame_id
      
      // Use the ID or index
      element.id = static_cast<int32_t>(i + 1);
      element.type = 1;  // Default type
      element.dynamic = is_moving;  // Set based on velocity
      
      // Set position - just copy from the circle
      element.pose.position = circle.center;
      
      // Set default orientation (identity quaternion)
      element.pose.orientation.w = 1.0;
      element.pose.orientation.x = 0.0;
      element.pose.orientation.y = 0.0;
      element.pose.orientation.z = 0.0;
      
      // Copy velocity from the circle
      element.velocity = circle.velocity;
      
      // Set size based on radius and default height
      element.size.x = 2.0 * circle.radius;  // Diameter in x
      element.size.y = 2.0 * circle.radius;  // Diameter in y
      element.size.z = default_height_;      // Default height
      
      // Set protective zone - if true_radius is available, use the difference
      // otherwise, use a small default value
      element.protective_zone = default_pz_;
      // Add to array
      elements_msg.elements.push_back(element);
    }
    
    // Process segment obstacles if needed
    for (size_t i = 0; i < msg->segments.size(); i++) {
      const auto& segment = msg->segments[i];
      
      // Calculate segment properties
      double center_x = (segment.first_point.x + segment.last_point.x) / 2.0;
      double center_y = (segment.first_point.y + segment.last_point.y) / 2.0;
      double center_z = (segment.first_point.z + segment.last_point.z) / 2.0;
      
      double dx = segment.last_point.x - segment.first_point.x;
      double dy = segment.last_point.y - segment.first_point.y;
      double length = std::sqrt(dx*dx + dy*dy);
      double angle = std::atan2(dy, dx);
      
      // Calculate average velocity (if available)
      geometry_msgs::msg::Vector3 velocity;
      velocity.x = (segment.first_velocity.x + segment.last_velocity.x) / 2.0;
      velocity.y = (segment.first_velocity.y + segment.last_velocity.y) / 2.0;
      velocity.z = (segment.first_velocity.z + segment.last_velocity.z) / 2.0;

      
      
      bool is_moving = (std::abs(velocity.x) > min_velocity_threshold_ || 
                       std::abs(velocity.y) > min_velocity_threshold_ ||
                       std::abs(velocity.z) > min_velocity_threshold_);
      
      // RCLCPP_INFO(this->get_logger(), "Segment %zu: Length: %.2f, Center (%.2f, %.2f, %.2f), Velocity (%.2f, %.2f, %.2f), Moving: %s", 
      //            i, length, center_x, center_y, center_z,
      //            velocity.x, velocity.y, velocity.z,
      //            is_moving ? "true" : "false");
      
      // Create element message for segment
      auto element = custom_msgs::msg::ElementCharacteristicsStamped();
      element.header.stamp = current_time;
      element.header.frame_id = frame_id_;
      
      // Use the ID or index (ensure unique IDs by offsetting by number of circles)
      element.id = static_cast<int32_t>(msg->circles.size() + i + 1);
      element.type = 2;  // Different type for segments
      element.dynamic = is_moving;
      
      // Set position at center of segment
      element.pose.position.x = center_x;
      element.pose.position.y = center_y;
      element.pose.position.z = center_z;
      
      // Set orientation based on segment angle
      // Convert the yaw angle to a quaternion (rotation around Z axis)
      double cy = std::cos(angle * 0.5);
      double sy = std::sin(angle * 0.5);
      element.pose.orientation.w = cy;
      element.pose.orientation.z = sy;
      element.pose.orientation.x = 0.0;
      element.pose.orientation.y = 0.0;
      
      // Set velocity
      element.velocity = velocity;
      
      // Set size
      element.size.x = length;      // Length
      element.size.y = 0.2;         // Small width for visualization
      element.size.z = default_height_;
      
      // Set protective zone
      element.protective_zone = default_pz_;  
      
      // Add to array
      elements_msg.elements.push_back(element);
    }
    
    // Publish elements
    if (true || !elements_msg.elements.empty()) {
      elements_publisher_->publish(elements_msg);
      // RCLCPP_INFO(this->get_logger(), "Published %zu elements (circles: %zu, segments: %zu)", 
      //            elements_msg.elements.size(), msg->circles.size(), msg->segments.size());
    } else {
      RCLCPP_WARN(this->get_logger(), "No elements to publish");
    }
  }

  // Member variables
  std::string frame_id_;
  double default_height_;
  double min_velocity_threshold_;
  double default_pz_;
  
  rclcpp::Publisher<custom_msgs::msg::ElementCharacteristicsArray>::SharedPtr elements_publisher_;
  rclcpp::Subscription<obstacle_detector::msg::Obstacles>::SharedPtr obstacles_subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<EnhancedObstacleConverterNode>());
  rclcpp::shutdown();
  return 0;
}