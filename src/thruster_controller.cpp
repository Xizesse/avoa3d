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
    this->declare_parameter<std::string>("topics.thrust_cmd", "/nest/thrusters/thrust_cmd");
    this->declare_parameter<std::string>("topics.odometry", "/nest/odometry");
    this->declare_parameter<std::string>("topics.cmd_vel", "/avoa/cmd_vel");

    // Declare parameters with default values for linear motion
    this->declare_parameter("kp_linear", 120.0);
    this->declare_parameter("ki_linear", 1.5);
    this->declare_parameter("kd_linear", 0.0);
    
    // New parameters for angular motion (yaw control)
    this->declare_parameter("kp_angular", 50.0);
    this->declare_parameter("ki_angular", 0.0);
    this->declare_parameter("kd_angular", 0.0);
    
    // Other parameters
    this->declare_parameter("dt", 0.1);
    this->declare_parameter("input_filter", 0.3);
    this->declare_parameter("max_integral_linear", 1000.0);
    this->declare_parameter("max_integral_angular", 100.0);

    std::string thrust_cmd_topic = this->get_parameter("topics.thrust_cmd").as_string();
    std::string odometry_topic = this->get_parameter("topics.odometry").as_string();
    std::string cmd_vel_topic = this->get_parameter("topics.cmd_vel").as_string();
    
    // Get parameter values
    kp_linear_ = this->get_parameter("kp_linear").as_double();
    ki_linear_ = this->get_parameter("ki_linear").as_double();
    kd_linear_ = this->get_parameter("kd_linear").as_double();
    
    // Get angular control parameters
    kp_angular_ = this->get_parameter("kp_angular").as_double();
    ki_angular_ = this->get_parameter("ki_angular").as_double();
    kd_angular_ = this->get_parameter("kd_angular").as_double();
    
    // Get other parameters
    dt_ = this->get_parameter("dt").as_double();
    input_filter_ = this->get_parameter("input_filter").as_double();
    max_integral_linear_ = this->get_parameter("max_integral_linear").as_double();
    max_integral_angular_ = this->get_parameter("max_integral_angular").as_double();
    
    std::cout << "================================================================" << std::endl;
    std::cout << "=============== THRUSTER CONTROLLER INITIALIZATION ============" << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << "Topics:" << std::endl;
    std::cout << "  - Thrust Command: " << thrust_cmd_topic << std::endl;
    std::cout << "  - Odometry: " << odometry_topic << std::endl;
    std::cout << "  - Command Velocity: " << cmd_vel_topic << std::endl;
    std::cout << "Control Parameters:" << std::endl;
    std::cout << "  - Linear control (P,I,D): ("
              << kp_linear_ << ", " << ki_linear_ << ", " << kd_linear_ << ")" << std::endl;
    std::cout << "  - Angular control (P,I,D): ("
              << kp_angular_ << ", " << ki_angular_ << ", " << kd_angular_ << ")" << std::endl;
    std::cout << "  - dt: " << dt_ << ", input_filter: " << input_filter_ << std::endl;
    std::cout << "================================================================\n" << std::endl;
    
    // Create a publisher for the ThrustCommand message
    thrust_publisher_ = this->create_publisher<nest_interfaces::msg::ThrustCommand>(
      thrust_cmd_topic, 10);
    
    // Subscribe to odometry for current velocity
    odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
      odometry_topic, 10, 
      std::bind(&ThrustCommandPublisher::odom_callback, this, std::placeholders::_1));
    
    // Subscribe to desired velocity
    desired_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
      cmd_vel_topic, 10, 
      std::bind(&ThrustCommandPublisher::desired_vel_callback, this, std::placeholders::_1));
    
    // Create a timer to publish messages at 10Hz (or as specified by the dt parameter)
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(static_cast<int>(dt_ * 1000)), 
      std::bind(&ThrustCommandPublisher::publish_command, this));
    
    // Initialize error tracking for PID
    prev_error_x_ = 0.0;
    prev_error_y_ = 0.0;
    prev_error_yaw_ = 0.0;  
    integral_x_ = 0.0;
    integral_y_ = 0.0;
    integral_yaw_ = 0.0;    
    
    RCLCPP_INFO(this->get_logger(), "Velocity Controller started with parameters:");
    RCLCPP_INFO(this->get_logger(), "  Linear control: kp=%.2f, ki=%.2f, kd=%.2f", 
                kp_linear_, ki_linear_, kd_linear_);
    RCLCPP_INFO(this->get_logger(), "  Angular control: kp=%.2f, ki=%.2f, kd=%.2f", 
                kp_angular_, ki_angular_, kd_angular_);
    RCLCPP_INFO(this->get_logger(), "  dt: %.2f, input_filter: %.2f", dt_, input_filter_);
    RCLCPP_INFO(this->get_logger(), "  max_integral_linear: %.2f, max_integral_angular: %.2f", 
                max_integral_linear_, max_integral_angular_);
  }

private:
  void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    // Store current velocity
    current_vel_x_ = msg->twist.twist.linear.x + 0.0*current_vel_x_;
    current_vel_y_ = msg->twist.twist.linear.y + 0.0*current_vel_y_;
    current_vel_z_ = msg->twist.twist.linear.z + 0.0*current_vel_z_;
    
    current_ang_vel_x_ = msg->twist.twist.angular.x + 0.0*current_ang_vel_x_;
    current_ang_vel_y_ = msg->twist.twist.angular.y + 0.0*current_ang_vel_y_;
    current_ang_vel_z_ = msg->twist.twist.angular.z + 0.0*current_ang_vel_z_;
    
    have_odom_ = true;
  }
  
  // Callback for desired velocity subscription
  void desired_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
  {
    // Store desired velocity from subscription with input filter
    desired_vel_x_ = (1.0 - input_filter_) * desired_vel_x_ + input_filter_ * msg->linear.x;
    desired_vel_y_ = (1.0 - input_filter_) * desired_vel_y_ + input_filter_ * msg->linear.y;
    desired_vel_z_ = (1.0 - input_filter_) * desired_vel_z_ + input_filter_ * msg->linear.z;
    desired_ang_vel_x_ = (1.0 - input_filter_) * desired_ang_vel_x_ + input_filter_ * msg->angular.x;
    desired_ang_vel_y_ = (1.0 - input_filter_) * desired_ang_vel_y_ + input_filter_ * msg->angular.y;
    desired_ang_vel_z_ = (1.0 - input_filter_) * desired_ang_vel_z_ + input_filter_ * msg->angular.z;
    
    have_desired_vel_ = true;
  }
  
  double compute_linear_pid(double setpoint, double current_value, 
                           double &prev_error, double &integral)
  {
    // Calculate error
    double error = setpoint - current_value;
    
    // Proportional term with gain scheduling for low speeds
    double p_term = kp_linear_ * error;
    if (std::abs(current_value) < 0.2) {
      p_term = kp_linear_/3.0 * error;
    }
    
    // Integral term (with enhanced anti-windup)
    // Reset integral when trying to stop and near zero velocity
    if (std::abs(setpoint) < 0.01 && std::abs(current_value) < 0.1) {
      integral = 0.0;  // Reset integral when stopping
    } else {
      integral += error * dt_;
      // Limit integral term to prevent windup
      if (integral > max_integral_linear_) integral = max_integral_linear_;
      if (integral < -max_integral_linear_) integral = -max_integral_linear_;
    }
    double i_term = ki_linear_ * integral;
    
    // Derivative term
    double derivative = (error - prev_error) / dt_;
    double d_term = kd_linear_ * derivative;
    
    // Update previous error
    prev_error = error;
    
    // Return control output
    return p_term + i_term + d_term;
  }
  
  double compute_angular_pid(double setpoint, double current_value, 
                            double &prev_error, double &integral)
  {
    // Calculate error
    double error = setpoint - current_value;
    
    // Proportional term (with optional gain scheduling for low angular speeds)
    double p_term = kp_angular_ * error;
    if (std::abs(current_value) < 0.1) {  // If we're rotating slowly
      p_term = kp_angular_/2.0 * error;   // Use gentler gains
    }
    
    // Integral term (with anti-windup)
    // Reset integral when trying to stop rotation and near zero angular velocity
    if (std::abs(setpoint) < 0.01 && std::abs(current_value) < 0.05) {
      integral = 0.0;  // Reset integral when stopping rotation
    } else {
      integral += error * dt_;
      // Limit integral term to prevent windup
      if (integral > max_integral_angular_) integral = max_integral_angular_;
      if (integral < -max_integral_angular_) integral = -max_integral_angular_;
    }
    double i_term = ki_angular_ * integral;
    
    // Derivative term
    double derivative = (error - prev_error) / dt_;
    double d_term = kd_angular_ * derivative;
    
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
    
    // Compute control outputs using PID for x and y (linear)
    thrust_msg.fx = compute_linear_pid(desired_vel_x_, current_vel_x_, prev_error_x_, integral_x_);
    thrust_msg.fy = compute_linear_pid(desired_vel_y_, current_vel_y_, prev_error_y_, integral_y_);
    
    // Compute yaw control (angular z)
    thrust_msg.mz = compute_angular_pid(desired_ang_vel_z_, current_ang_vel_z_, prev_error_yaw_, integral_yaw_);
    
    thrust_msg.fz = 0.0;
    thrust_msg.mx = 0.0;
    thrust_msg.my = 0.0;
    
    thrust_publisher_->publish(thrust_msg);
    
  }

  // Publishers
  rclcpp::Publisher<nest_interfaces::msg::ThrustCommand>::SharedPtr thrust_publisher_;
  
  // Subscribers
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr desired_vel_sub_;
  
  // Timer
  rclcpp::TimerBase::SharedPtr timer_;
  
  // Linear control parameters
  double kp_linear_;
  double ki_linear_;
  double kd_linear_;
  
  // Angular control parameters (for yaw)
  double kp_angular_;
  double ki_angular_;
  double kd_angular_;
  
  // Other parameters
  double dt_;
  double input_filter_;
  double max_integral_linear_;
  double max_integral_angular_;
  
  // PID error tracking for linear motion
  double prev_error_x_;
  double prev_error_y_;
  double integral_x_;
  double integral_y_;
  
  // PID error tracking for angular motion (yaw)
  double prev_error_yaw_;
  double integral_yaw_;
  
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

