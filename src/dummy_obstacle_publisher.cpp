#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <cmath>
#include <algorithm>

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
        this->declare_parameter<std::string>("agent_frame", "base_link");
        this->declare_parameter<std::string>("odometry_topic", "/nest/odometry");
        this->declare_parameter<double>("publish_rate", 10.0);
        this->declare_parameter<std::string>("obstacle_odometry_topic", "/model/obstacle/odometry");
        
        // Obstacle initial position in world coordinates
        this->declare_parameter<double>("obstacle_initial_x", 5.0);
        this->declare_parameter<double>("obstacle_initial_y", 10.0);
        this->declare_parameter<double>("obstacle_initial_z", 0.0);
        
        // Obstacle velocity in world coordinates
        this->declare_parameter<double>("obstacle_vel_x", -0.0);
        this->declare_parameter<double>("obstacle_vel_y", -0.5);
        this->declare_parameter<double>("obstacle_vel_z", 0.0);
        
        // Obstacle properties
        this->declare_parameter<double>("obstacle_radius", 0.5);
        this->declare_parameter<double>("obstacle_height", 1.0);
        this->declare_parameter<double>("protective_zone", 0.2);
        
        // Get parameters
        agent_frame_ = this->get_parameter("agent_frame").as_string();
        odometry_topic_ = this->get_parameter("odometry_topic").as_string();
        obstacle_odometry_topic_ = this->get_parameter("obstacle_odometry_topic").as_string();
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
        RCLCPP_INFO(this->get_logger(), "Obstacle odometry topic: %s", obstacle_odometry_topic_.c_str());
        RCLCPP_INFO(this->get_logger(), "Obstacle initial position: (%.2f, %.2f, %.2f)", 
                    obstacle_initial_pos_.x, obstacle_initial_pos_.y, obstacle_initial_pos_.z);
        RCLCPP_INFO(this->get_logger(), "Obstacle velocity: (%.2f, %.2f, %.2f)", 
                    obstacle_velocity_.x, obstacle_velocity_.y, obstacle_velocity_.z);
        RCLCPP_INFO(this->get_logger(), "Obstacle dynamic: %s", obstacle_dynamic_ ? "true" : "false");
        RCLCPP_INFO(this->get_logger(), "Obstacle radius: %.2f, height: %.2f", 
                    obstacle_radius_, obstacle_height_);
        RCLCPP_INFO(this->get_logger(), "Publishing rate: %.1f Hz", publish_rate_);
        
        // Create publishers
        elements_publisher_ = this->create_publisher<avoa3d::msg::ElementCharacteristicsArray>(
            "/element_tracking/elements", 10);
        
        // Add obstacle odometry publisher for metrics recording (world frame)
        obstacle_odometry_publisher_ = this->create_publisher<nav_msgs::msg::Odometry>(
            obstacle_odometry_topic_, 10);
        
        // Create odometry subscriber (always needed for agent frame transformation)
        odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
            odometry_topic_, 10,
            std::bind(&DummyObstaclePublisher::odometry_callback, this, std::placeholders::_1));
        
        // Create timer
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(static_cast<int>(1000.0 / publish_rate_)),
            std::bind(&DummyObstaclePublisher::timer_callback, this));
        
        RCLCPP_INFO(this->get_logger(), "Publishing to: /element_tracking/elements (agent frame)");
        RCLCPP_INFO(this->get_logger(), "Publishing to: %s (world frame)", obstacle_odometry_topic_.c_str());
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
        // We need odometry for all transformations to agent frame
        if (!has_odometry_) {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                                "No odometry data received yet");
            return;
        }
        
        // Get current time
        auto current_time = this->get_clock()->now();
        
        // Calculate current obstacle position in world coordinates
        geometry_msgs::msg::Point current_obstacle_world_pos = calculate_current_obstacle_position(current_time);
        
        // ALWAYS transform to agent frame (as required)
        geometry_msgs::msg::Point obstacle_agent_pos = transform_to_vehicle_frame(
            current_obstacle_world_pos, 
            latest_odometry_.pose.pose.position,
            latest_odometry_.pose.pose.orientation);
        
        // Verify transformation makes sense (simple sanity check)
        double distance_world = sqrt(
            pow(current_obstacle_world_pos.x - latest_odometry_.pose.pose.position.x, 2) +
            pow(current_obstacle_world_pos.y - latest_odometry_.pose.pose.position.y, 2) +
            pow(current_obstacle_world_pos.z - latest_odometry_.pose.pose.position.z, 2)
        );
        double distance_agent = sqrt(
            pow(obstacle_agent_pos.x, 2) + pow(obstacle_agent_pos.y, 2) + pow(obstacle_agent_pos.z, 2)
        );
        
        // Distances should be approximately equal (sanity check)
        if (std::abs(distance_world - distance_agent) > 0.1) {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
                                "Transformation sanity check failed: world_dist=%.3f, agent_dist=%.3f", 
                                distance_world, distance_agent);
        }
        
        // Create element characteristics array
        auto elements_msg = avoa3d::msg::ElementCharacteristicsArray();
        
        // Create the obstacle element
        auto element = avoa3d::msg::ElementCharacteristicsStamped();
        element.header.stamp = current_time;
        element.header.frame_id = agent_frame_;  // Always agent frame as required
        
        // Set position in agent frame
        element.pose.position = obstacle_agent_pos;
        
        // Set obstacle properties
        element.id = 1;
        element.type = 1;  // Circular obstacle type
        element.dynamic = obstacle_dynamic_;
        
        // Set default orientation (identity quaternion)
        element.pose.orientation.w = 1.0;
        element.pose.orientation.x = 0.0;
        element.pose.orientation.y = 0.0;
        element.pose.orientation.z = 0.0;
        
        // Set velocity (transform to agent frame for dynamic obstacles)
        if (obstacle_dynamic_) {
            // For dynamic obstacles, transform velocity to agent frame
            // (This is a simplified approach - full implementation would need velocity transformation)
            element.velocity = obstacle_velocity_;
        } else {
            // Static obstacles have zero velocity
            element.velocity.x = 0.0;
            element.velocity.y = 0.0;
            element.velocity.z = 0.0;
        }
        
        // Set size based on radius and height
        element.size.x = 2.0 * obstacle_radius_;  // Diameter in x
        element.size.y = 2.0 * obstacle_radius_;  // Diameter in y
        element.size.z = obstacle_height_;        // Height
        
        // Set additional properties
        element.radius_std = 0.05;  // Small standard deviation
        element.protective_zone = protective_zone_;
        
        // Add to array
        elements_msg.elements.push_back(element);
        
        // Publish element characteristics (agent frame)
        elements_publisher_->publish(elements_msg);
        
        // Create and publish obstacle odometry message for metrics (world frame)
        auto obstacle_odom = nav_msgs::msg::Odometry();
        obstacle_odom.header.stamp = current_time;
        obstacle_odom.header.frame_id = "map";  // World frame for metrics
        obstacle_odom.child_frame_id = "obstacle";
        
        // Position in world coordinates (no transformation)
        obstacle_odom.pose.pose.position = current_obstacle_world_pos;
        
        // Default orientation (identity quaternion)
        obstacle_odom.pose.pose.orientation.w = 1.0;
        obstacle_odom.pose.pose.orientation.x = 0.0;
        obstacle_odom.pose.pose.orientation.y = 0.0;
        obstacle_odom.pose.pose.orientation.z = 0.0;
        
        // Velocity in world coordinates
        obstacle_odom.twist.twist.linear.x = obstacle_velocity_.x;
        obstacle_odom.twist.twist.linear.y = obstacle_velocity_.y;
        obstacle_odom.twist.twist.linear.z = obstacle_velocity_.z;
        
        // Publish obstacle odometry (world frame)
        obstacle_odometry_publisher_->publish(obstacle_odom);
        
        // Debug output (throttled)
        RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                             "Agent pos: (%.2f, %.2f, %.2f), Obstacle world: (%.2f, %.2f, %.2f) -> agent: (%.2f, %.2f, %.2f)",
                             latest_odometry_.pose.pose.position.x, latest_odometry_.pose.pose.position.y, latest_odometry_.pose.pose.position.z,
                             current_obstacle_world_pos.x, current_obstacle_world_pos.y, current_obstacle_world_pos.z,
                             obstacle_agent_pos.x, obstacle_agent_pos.y, obstacle_agent_pos.z);
        
        RCLCPP_DEBUG_THROTTLE(this->get_logger(), *this->get_clock(), 5000,
                             "Published: element_tracking (agent frame) + obstacle odometry (world frame)");
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
        // Step 1: Translate to vehicle origin (world coordinates)
        double dx = world_point.x - vehicle_position.x;
        double dy = world_point.y - vehicle_position.y;
        double dz = world_point.z - vehicle_position.z;
        
        // Step 2: Apply inverse rotation to align with vehicle frame
        // Convert quaternion to rotation matrix (inverse/conjugate)
        double qw = vehicle_orientation.w;
        double qx = vehicle_orientation.x;
        double qy = vehicle_orientation.y;
        double qz = vehicle_orientation.z;
        
        // Normalize quaternion (safety check)
        double norm = sqrt(qw*qw + qx*qx + qy*qy + qz*qz);
        if (norm > 0.0001) {
            qw /= norm;
            qx /= norm;
            qy /= norm;
            qz /= norm;
        }
        
        // For inverse rotation, use conjugate quaternion (-x, -y, -z, w)
        double qx_inv = -qx;
        double qy_inv = -qy;
        double qz_inv = -qz;
        double qw_inv = qw;
        
        // Apply inverse rotation using rotation matrix from conjugate quaternion
        geometry_msgs::msg::Point relative_point;
        
        // Rotation matrix elements for inverse transform
        double r11 = 1 - 2*(qy_inv*qy_inv + qz_inv*qz_inv);
        double r12 = 2*(qx_inv*qy_inv - qw_inv*qz_inv);
        double r13 = 2*(qx_inv*qz_inv + qw_inv*qy_inv);
        
        double r21 = 2*(qx_inv*qy_inv + qw_inv*qz_inv);
        double r22 = 1 - 2*(qx_inv*qx_inv + qz_inv*qz_inv);
        double r23 = 2*(qy_inv*qz_inv - qw_inv*qx_inv);
        
        double r31 = 2*(qx_inv*qz_inv - qw_inv*qy_inv);
        double r32 = 2*(qy_inv*qz_inv + qw_inv*qx_inv);
        double r33 = 1 - 2*(qx_inv*qx_inv + qy_inv*qy_inv);
        
        // Apply rotation
        relative_point.x = r11 * dx + r12 * dy + r13 * dz;
        relative_point.y = r21 * dx + r22 * dy + r23 * dz;
        relative_point.z = r31 * dx + r32 * dy + r33 * dz;
        
        return relative_point;
    }

    // Member variables
    std::string agent_frame_;
    std::string odometry_topic_;
    std::string obstacle_odometry_topic_;
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
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr obstacle_odometry_publisher_;
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