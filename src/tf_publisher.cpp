#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "tf2_ros/static_transform_broadcaster.h"
#include <tf2/LinearMath/Quaternion.h>
#include "geometry_msgs/msg/transform_stamped.hpp"

using namespace std::chrono_literals;

class TfBroadcasterNode : public rclcpp::Node
{
public:
  TfBroadcasterNode()
  : Node("tf_broadcaster_node")
  {
    // Create transform broadcasters
    tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);
    static_tf_broadcaster_ = std::make_shared<tf2_ros::StaticTransformBroadcaster>(this);
    
    // Subscribe to agent odometry for dynamic transform
    agent_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/model/agente/odometry", 10, 
      std::bind(&TfBroadcasterNode::agent_odometry_callback, this, std::placeholders::_1));
      
    // Publish static transforms once at startup
    publish_static_transforms();
    
    // Timer for checking if transforms need republishing
    timer_ = this->create_wall_timer(100ms, std::bind(&TfBroadcasterNode::timer_callback, this));
    
    RCLCPP_INFO(this->get_logger(), "TF Broadcaster Node initialized");
  }

private:
  void publish_static_transforms()
  {
    //! map to odom static transform from 
    geometry_msgs::msg::TransformStamped map_to_odom;
    map_to_odom.header.stamp = this->get_clock()->now();
    map_to_odom.header.frame_id = "map";
    map_to_odom.child_frame_id = "odom";
    
    // Identity transform (map = odom initially)
    map_to_odom.transform.translation.x = 0.0;
    map_to_odom.transform.translation.y = 0.0;
    map_to_odom.transform.translation.z = 0.0;
    
    tf2::Quaternion q_identity;
    q_identity.setRPY(0.0, 0.0, 0.0);
    map_to_odom.transform.rotation.x = q_identity.x();
    map_to_odom.transform.rotation.y = q_identity.y();
    map_to_odom.transform.rotation.z = q_identity.z();
    map_to_odom.transform.rotation.w = q_identity.w();
    
    // Publish static transform
    static_tf_broadcaster_->sendTransform(map_to_odom);
    
    RCLCPP_INFO(this->get_logger(), "Published static transform: map -> odom");
  }
  
  void publish_odom_to_base_link()
  {
    if (!has_agent_odometry_) {
      return;  // No odometry data yet
    }
    
    //! odom to base_linkTransform from  (using agent odometry)
    geometry_msgs::msg::TransformStamped odom_to_base_link;
    odom_to_base_link.header.stamp = this->get_clock()->now();
    odom_to_base_link.header.frame_id = "odom";
    odom_to_base_link.child_frame_id = "base_link";
    
    // Copy values from agent odometry
    odom_to_base_link.transform.translation.x = agent_odometry_.pose.pose.position.x;
    odom_to_base_link.transform.translation.y = agent_odometry_.pose.pose.position.y;
    odom_to_base_link.transform.translation.z = agent_odometry_.pose.pose.position.z;
    odom_to_base_link.transform.rotation = agent_odometry_.pose.pose.orientation;
    
    // Publish transform
    tf_broadcaster_->sendTransform(odom_to_base_link);
  }
  
  void publish_base_link_to_agent()
  {
    // !base_link to agentIdentity transform from  (to maintain compatibility)
    geometry_msgs::msg::TransformStamped base_link_to_agent;
    base_link_to_agent.header.stamp = this->get_clock()->now();
    base_link_to_agent.header.frame_id = "base_link";
    base_link_to_agent.child_frame_id = "agent";
    
    // Identity transform
    base_link_to_agent.transform.translation.x = 0.0;
    base_link_to_agent.transform.translation.y = 0.0;
    base_link_to_agent.transform.translation.z = 0.0;
    
    tf2::Quaternion q_identity;
    q_identity.setRPY(0.0, 0.0, 0.0);
    base_link_to_agent.transform.rotation.x = q_identity.x();
    base_link_to_agent.transform.rotation.y = q_identity.y();
    base_link_to_agent.transform.rotation.z = q_identity.z();
    base_link_to_agent.transform.rotation.w = q_identity.w();
    
    // Publish transform
    tf_broadcaster_->sendTransform(base_link_to_agent);
  }

  void agent_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    agent_odometry_ = *msg;
    has_agent_odometry_ = true;
  }
  
  void timer_callback()
  {
    publish_odom_to_base_link();
    publish_base_link_to_agent();
  }

  // Members
  std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  std::shared_ptr<tf2_ros::StaticTransformBroadcaster> static_tf_broadcaster_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr agent_odometry_subscriber_;
  rclcpp::TimerBase::SharedPtr timer_;
  
  nav_msgs::msg::Odometry agent_odometry_;
  bool has_agent_odometry_ = false;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TfBroadcasterNode>());
  rclcpp::shutdown();
  return 0;
}