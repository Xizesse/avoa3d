#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "avoa3d/msg/element_characteristics_stamped.hpp"
#include "avoa3d/msg/element_characteristics_array.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "geometry_msgs/msg/point.hpp"
#include "geometry_msgs/msg/quaternion.hpp"

using namespace std::chrono_literals;

class DummyObstaclePublisher : public rclcpp::Node
{
public:
    DummyObstaclePublisher() : Node("dummy_obstacle_publisher")
    {
        // Declare parameters
        this->declare_parameter<std::string>("agent_frame", "agente");
        this->declare_parameter<std::string>("odometry_topic", "/model/agente/odometry");
        this->declare_parameter<double>("publish_rate", 10.0);
        
        // Obstacle initial position in world coordinates
        this->declare_parameter<double>("obstacle_initial_x", 5.0);
        this->declare_parameter<double>("obstacle_initial_y", 0.0);
        this->declare_parameter<double>("obstacle_initial_z", 0.0);
        
        // Obstacle velocity in world coordinates
        this->declare_parameter<double>("obstacle_vel_x", -0.5);
        this->declare_parameter<double>("obstacle_vel_y", 0.0);
        this->declare_parameter<double>("obstacle_vel_z", 0.0);
        
        // Obstacle properties
        this->declare_parameter<double>("obstacle_radius", 0.5);
        this->declare_parameter<double>("obstacle_height", 1.0);
        this->declare_parameter<double>("protective_zone", 0.2);
        
        // Get parameters
        agent_frame_ = this->get_parameter("agent_frame").as_string();
        odometry_topic_ = this->get_parameter("odometry_topic").as_string();
        publish_rate_ = this->get_parameter("publish_rate").as_double();
        
        // Obstacle initial position in world coordinates
        obstacle_initial_pos_.x = this->get_parameter("obstacle_initial_x").as_double();
        obstacle_initial_pos_.y = this->get_parameter("obstacle_initial_y").as_double();
        obstacle_initial_pos_.z = this->get_parameter("obstacle_initial_z").as_double();
        
        // Obstacle velocity in world coordinates
        obstacle_velocity_.x = this->get_parameter("obstacle_vel_x").as_double();
        obstacle_velocity_.y = this->get_parameter("obstacle_vel_y").as_double();
        obstacle_velocity_.z = this->get_parameter("obstacle_vel_z").as_double();
        
        // Other properties
        obstacle_radius_ = this->get_parameter("obstacle_radius").as_double();
        obstacle_height_ = this->get_parameter("obstacle_height").as_double();
        protective_zone_ = this->get_parameter("protective_zone").as_double();
        
        // Check if obstacle is dynamic
        obstacle_dynamic_ = (std::abs(obstacle_velocity_.x) > 0.001 || 
                           std::abs(obstacle_velocity_.y) > 0.001 || 
                           std::abs(obstacle_velocity_.z) > 0.001);
        
        // Store start time for motion calculation
        start_time_ = this->get_clock()->now();
        
        RCLCPP_INFO(this->get_logger(), "Starting Dummy Obstacle Publisher Node");
        RCLCPP_INFO(this->get_logger(), "Agent frame: %s", agent_frame_.c_str());
        RCLCPP_INFO(this->get_logger(), "Odometry topic: %s", odometry_topic_.c_str());
        RCLCPP_INFO(this->get_logger(), "Obstacle initial position: (%.2f, %.2f, %.2f)", 
                    obstacle_initial_pos_.x, obstacle_initial_pos_.y, obstacle_initial_pos_.z);
        RCLCPP_INFO(this->get_logger(), "Obstacle velocity: (%.2f, %.2f, %.2f)", 
                    obstacle_velocity_.x, obstacle_velocity_.y, obstacle_velocity_.z);
        RCLCPP_INFO(this->get_logger(), "Obstacle dynamic: %s", obstacle_dynamic_ ? "true" : "false");
        RCLCPP_INFO(this->get_logger(), "Obstacle radius: %.2f, height: %.2f", 
                    obstacle_radius_, obstacle_height_);
        RCLCPP_INFO(this->get_logger(), "Publishing rate: %.1f Hz", publish_rate_);
        
        // Create publisher
        elements_publisher_ = this->create_publisher<avoa3d::msg::ElementCharacteristicsArray>(
            "/element_tracking/elements", 10);
        
        // Create odometry subscriber
        odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
            odometry_topic_, 10,
            std::bind(&DummyObstaclePublisher::odometry_callback, this, std::placeholders::_1));
        
        // Create timer
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(static_cast<int>(1000.0 / publish_rate_)),
            std::bind(&DummyObstaclePublisher::timer_callback, this));
        
        RCLCPP_INFO(this->get_logger(), "Publishing to: /element_tracking/elements");
        RCLCPP_INFO(this->get_logger(), "Subscribed to: %s", odometry_topic_.c_str());
    }

private:
    void odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        latest_odometry_ = *msg;
        has_odometry_ = true;
        RCLCPP_DEBUG(this->get_logger(), "Received odometry update");
    }
    
    void timer_callback()
    {
        if (!has_odometry_) {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                                "No odometry data received yet");
            return;
        }
        
        // Get current time
        auto current_time = this->get_clock()->now();
        
        // Calculate current obstacle position in world coordinates
        geometry_msgs::msg::Point current_obstacle_world_pos = calculate_current_obstacle_position(current_time);
        
        // Calculate obstacle position relative to vehicle
        geometry_msgs::msg::Point obstacle_relative_pos = transform_to_vehicle_frame(
            current_obstacle_world_pos, 
            latest_odometry_.pose.pose.position,
            latest_odometry_.pose.pose.orientation);
        
        // Create element characteristics array
        auto elements_msg = avoa3d::msg::ElementCharacteristicsArray();
        
        // Create the obstacle element
        auto element = avoa3d::msg::ElementCharacteristicsStamped();
        element.header.stamp = current_time;
        element.header.frame_id = agent_frame_;
        
        // Set obstacle properties
        element.id = 1;
        element.type = 1;  // Circular obstacle type
        element.dynamic = obstacle_dynamic_;
        
        // Set position in agent frame
        element.pose.position = obstacle_relative_pos;
        
        // Set default orientation (identity quaternion)
        element.pose.orientation.w = 1.0;
        element.pose.orientation.x = 0.0;
        element.pose.orientation.y = 0.0;
        element.pose.orientation.z = 0.0;
        
        // Set velocity (transform velocity to vehicle frame if needed)
        element.velocity = obstacle_velocity_;
        
        // Set size based on radius and height
        element.size.x = 2.0 * obstacle_radius_;  // Diameter in x
        element.size.y = 2.0 * obstacle_radius_;  // Diameter in y
        element.size.z = obstacle_height_;        // Height
        
        // Set additional properties
        element.radius_std = 0.05;  // Small standard deviation
        element.protective_zone = protective_zone_;
        
        // Add to array
        elements_msg.elements.push_back(element);
        
        // Publish
        elements_publisher_->publish(elements_msg);
        
        // Debug output (throttled)
        RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                             "Obstacle world pos: (%.2f, %.2f, %.2f) -> agent pos: (%.2f, %.2f, %.2f)",
                             current_obstacle_world_pos.x, current_obstacle_world_pos.y, current_obstacle_world_pos.z,
                             obstacle_relative_pos.x, obstacle_relative_pos.y, obstacle_relative_pos.z);
    }
    
    geometry_msgs::msg::Point calculate_current_obstacle_position(const rclcpp::Time& current_time)
    {
        geometry_msgs::msg::Point current_pos = obstacle_initial_pos_;
        
        if (obstacle_dynamic_) {
            // Calculate elapsed time in seconds
            double elapsed_time = (current_time - start_time_).seconds();
            
            // Update position based on velocity
            current_pos.x = obstacle_initial_pos_.x + obstacle_velocity_.x * elapsed_time;
            current_pos.y = obstacle_initial_pos_.y + obstacle_velocity_.y * elapsed_time;
            current_pos.z = obstacle_initial_pos_.z + obstacle_velocity_.z * elapsed_time;
        }
        
        return current_pos;
    }
    
    geometry_msgs::msg::Point transform_to_vehicle_frame(
        const geometry_msgs::msg::Point& world_point,
        const geometry_msgs::msg::Point& vehicle_position,
        const geometry_msgs::msg::Quaternion& vehicle_orientation)
    {
        // Translate: obstacle position relative to vehicle position in world frame
        double dx = world_point.x - vehicle_position.x;
        double dy = world_point.y - vehicle_position.y;
        double dz = world_point.z - vehicle_position.z;
        
        // Convert quaternion to rotation matrix (inverse rotation)
        // For inverse rotation, we use the conjugate of the quaternion
        double w = vehicle_orientation.w;
        double x = -vehicle_orientation.x;  // Conjugate
        double y = -vehicle_orientation.y;  // Conjugate
        double z = -vehicle_orientation.z;  // Conjugate
        
        // Apply inverse rotation to get position in vehicle frame
        geometry_msgs::msg::Point relative_point;
        
        // Rotation matrix multiplication for inverse transform
        relative_point.x = (1 - 2*(y*y + z*z)) * dx + (2*(x*y + w*z)) * dy + (2*(x*z - w*y)) * dz;
        relative_point.y = (2*(x*y - w*z)) * dx + (1 - 2*(x*x + z*z)) * dy + (2*(y*z + w*x)) * dz;
        relative_point.z = (2*(x*z + w*y)) * dx + (2*(y*z - w*x)) * dy + (1 - 2*(x*x + y*y)) * dz;
        
        return relative_point;
    }

    // Member variables
    std::string agent_frame_;
    std::string odometry_topic_;
    double publish_rate_;
    
    // Obstacle properties
    geometry_msgs::msg::Point obstacle_initial_pos_;
    geometry_msgs::msg::Vector3 obstacle_velocity_;
    double obstacle_radius_;
    double obstacle_height_;
    bool obstacle_dynamic_;
    double protective_zone_;
    rclcpp::Time start_time_;
    
    // State
    nav_msgs::msg::Odometry latest_odometry_;
    bool has_odometry_ = false;
    
    // ROS2 components
    rclcpp::Publisher<avoa3d::msg::ElementCharacteristicsArray>::SharedPtr elements_publisher_;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odometry_subscriber_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DummyObstaclePublisher>());
    rclcpp::shutdown();
    return 0;
}