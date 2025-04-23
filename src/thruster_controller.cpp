#include <rclcpp/rclcpp.hpp>
#include <nest_interfaces/msg/thrust_command.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <cmath>
#include <chrono>

using namespace std::chrono_literals;

class ThrustCommandPublisher : public rclcpp::Node
{
public:
  ThrustCommandPublisher() : Node("thruster_controller")
  {
    // Create a publisher for the ThrustCommand message
    thrust_publisher_ = this->create_publisher<nest_interfaces::msg::ThrustCommand>(
      "/nest/thrusters/thrust_cmd", 10);
    
    // Subscribe to odometry for current velocity
    odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/nest/odometry", 10, 
      std::bind(&ThrustCommandPublisher::odom_callback, this, std::placeholders::_1));
    
    // Subscribe to desired velocity instead of simulating it
    desired_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
      "avoa/cmd_vel", 10,  // Changed topic name to /nest/cmd_vel
      std::bind(&ThrustCommandPublisher::desired_vel_callback, this, std::placeholders::_1));
    
    // Create a timer to publish messages at 10Hz
    timer_ = this->create_wall_timer(
      100ms, std::bind(&ThrustCommandPublisher::publish_command, this));
    
    // Initialize PID gains
    kp_linear_ = 100.0;  // Proportional gain for linear velocity
    ki_linear_ = 5.0;    // Integral gain for linear velocity
    kd_linear_ = 10.0;   // Derivative gain for linear velocity
    
    // Initialize error tracking for PID
    prev_error_x_ = 0.0;  // Add error tracking for x
    prev_error_y_ = 0.0;
    integral_x_ = 0.0;    // Add integral tracking for x
    integral_y_ = 0.0;
    
    // Control loop time step
    dt_ = 0.1; // 10Hz = 0.1s
    
    // Initialize desired velocity
    desired_vel_x_ = 0.0;
    desired_vel_y_ = 0.0;
    
    RCLCPP_INFO(this->get_logger(), "Velocity Controller started");
  }

private:
  void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    // Store current velocity
    current_vel_x_ = msg->twist.twist.linear.x;
    current_vel_y_ = msg->twist.twist.linear.y;
    current_vel_z_ = msg->twist.twist.linear.z;
    
    current_ang_vel_x_ = msg->twist.twist.angular.x;
    current_ang_vel_y_ = msg->twist.twist.angular.y;
    current_ang_vel_z_ = msg->twist.twist.angular.z;
    
    have_odom_ = true;
  }
  
  // New callback for desired velocity subscription
  void desired_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
  {
    // Store desired velocity from subscription
    desired_vel_x_ = msg->linear.x;
    desired_vel_y_ = msg->linear.y;
    desired_vel_z_ = msg->linear.z;
    
    desired_ang_vel_x_ = msg->angular.x;
    desired_ang_vel_y_ = msg->angular.y;
    desired_ang_vel_z_ = msg->angular.z;
    
    have_desired_vel_ = true;
  }
  
  double compute_pid_control(double setpoint, double current_value, 
                             double &prev_error, double &integral)
  {
    // Calculate error
    double error = setpoint - current_value;
    
    // Proportional term
    double p_term = kp_linear_ * error;
    
    // Integral term (with anti-windup)
    integral += error * dt_;
    // Limit integral term to prevent windup
    double max_integral = 50.0;
    if (integral > max_integral) integral = max_integral;
    if (integral < -max_integral) integral = -max_integral;
    double i_term = ki_linear_ * integral;
    
    // Derivative term
    double derivative = (error - prev_error) / dt_;
    double d_term = kd_linear_ * derivative;
    
    // Update previous error
    prev_error = error;
    
    // Return control output
    return p_term + i_term + d_term;
  }
  
  void publish_command()
  {
    // Create a new ThrustCommand message
    auto thrust_msg = nest_interfaces::msg::ThrustCommand();
    
    // Skip control computation if no odometry data yet
    if (!have_odom_) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                           "No odometry data received yet");
      return;
    }
    
    // Skip control computation if no desired velocity data yet
    if (!have_desired_vel_) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                           "No desired velocity received yet");
      return;
    }
    
    // Compute control outputs using PID for both x and y
    thrust_msg.fx = compute_pid_control(desired_vel_x_, current_vel_x_, prev_error_x_, integral_x_);
    thrust_msg.fy = compute_pid_control(desired_vel_y_, current_vel_y_, prev_error_y_, integral_y_);
    
    // Set other thrust commands to zero (for now)
    thrust_msg.fz = 0.0;
    thrust_msg.mx = 0.0;
    thrust_msg.my = 0.0;
    thrust_msg.mz = 0.0;
    
    // Publish the thrust command
    thrust_publisher_->publish(thrust_msg);
    
    // Log debug information
    RCLCPP_INFO(this->get_logger(),
      "Desired vel_x: %.2f, Current vel_x: %.2f, Thrust_x: %.2f",
      desired_vel_x_, current_vel_x_, thrust_msg.fx);
    RCLCPP_INFO(this->get_logger(),
      "Desired vel_y: %.2f, Current vel_y: %.2f, Thrust_y: %.2f",
      desired_vel_y_, current_vel_y_, thrust_msg.fy);
  }

  // Publishers
  rclcpp::Publisher<nest_interfaces::msg::ThrustCommand>::SharedPtr thrust_publisher_;
  
  // Subscribers
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr desired_vel_sub_;
  
  // Timer
  rclcpp::TimerBase::SharedPtr timer_;
  
  // Control parameters
  double kp_linear_;
  double ki_linear_;
  double kd_linear_;
  double dt_;
  
  // PID error tracking
  double prev_error_x_;  // Add error tracking for x
  double prev_error_y_;
  double integral_x_;    // Add integral tracking for x
  double integral_y_;
  
  // Current velocity tracking
  double current_vel_x_ = 0.0;
  double current_vel_y_ = 0.0;
  double current_vel_z_ = 0.0;
  double current_ang_vel_x_ = 0.0;
  double current_ang_vel_y_ = 0.0;
  double current_ang_vel_z_ = 0.0;
  
  // Desired velocity from subscription
  double desired_vel_x_ = 0.0;
  double desired_vel_y_ = 0.0;
  double desired_vel_z_ = 0.0;
  double desired_ang_vel_x_ = 0.0;
  double desired_ang_vel_y_ = 0.0;
  double desired_ang_vel_z_ = 0.0;
  
  // State flags
  bool have_odom_ = false;
  bool have_desired_vel_ = false;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<ThrustCommandPublisher>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}