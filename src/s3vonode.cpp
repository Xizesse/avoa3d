#include <chrono>
#include <memory>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
//#include "custom_msgs/msg/element_characteristics_array.hpp"
#include "s3vo/msg/element_characteristics_stamped.hpp"
#include "s3vo/msg/element_characteristics_array.hpp"
#include "s3vo/velocity_sample.hpp"
#include "s3vo/sample_evaluator.hpp"
#include "s3vo/sample_visualizer.hpp"
#include "s3vo/sample_generator.hpp"
#include "s3vo/motion_params.hpp"

using namespace std::chrono_literals;
using s3vo::VelocitySample;
using s3vo::MotionParameters;

class S3VO : public rclcpp::Node
{
public:
    S3VO() : Node("s3vonode", rclcpp::NodeOptions())
    {
        
        // Frame parameters
        this->declare_parameter("fixed_frame", "map");
        this->declare_parameter("agent_frame", "agent");
        
        // Topic parameters
        this->declare_parameter("topics.desired_vel", "/model/agente/desired_vel");
        this->declare_parameter("topics.cmd_vel", "/s3vo/cmd_vel");
        this->declare_parameter("topics.odometry", "/nest/odometry");
        this->declare_parameter("topics.element_tracking", "/element_tracking/elements");
        this->declare_parameter("topics.goal_odometry", "/model/goal/odometry");
        
        
        // S3VO parameters
        this->declare_parameter("kinematic_mode", "holonomic");
        this->declare_parameter("vehicle_radius", 0.0);
        this->declare_parameter("heading_weight", 0.0);
        this->declare_parameter("abs_weight", 0.0);
        this->declare_parameter("danger_weight", 0.0);
        this->declare_parameter("momentum_weight", 0.1);
        this->declare_parameter("linear_matching_weight", 0.0);
        this->declare_parameter("twist_matching_weight", 0.0);
        
        // Motion parameters
        this->declare_parameter("a_x_max", 0.0);
        this->declare_parameter("a_y_max", 0.0);
        this->declare_parameter("a_z_max", 0.0);
        this->declare_parameter("a_roll_max", 0.0);
        this->declare_parameter("a_pitch_max", 0.0);
        this->declare_parameter("a_yaw_max", 0.0);
        this->declare_parameter("v_x_max", 0.0);
        this->declare_parameter("v_y_max", 0.0);
        this->declare_parameter("v_z_max", 0.0);
        this->declare_parameter("w_roll_max", 0.0);
        this->declare_parameter("w_pitch_max", 0.0);
        this->declare_parameter("w_yaw_max", 0.0);
        this->declare_parameter("delta_t", 0.0);
        this->declare_parameter("num_samples", 0);
        this->declare_parameter("time_to_collision_threshold", 3000.0);
        this->declare_parameter("radius_threshold", 0.2);
        this->declare_parameter("watchdog_timeout", 1.0);
        
        // Log parameters
        std::string fixed_frame = this->get_parameter("fixed_frame").as_string();
        std::string agent_frame = this->get_parameter("agent_frame").as_string();

        std::string kinematic_mode = this->get_parameter("kinematic_mode").as_string();
        
        double vehicle_radius = this->get_parameter("vehicle_radius").as_double();
        double heading_weight = this->get_parameter("heading_weight").as_double();
        double abs_weight = this->get_parameter("abs_weight").as_double();
        double danger_weight = this->get_parameter("danger_weight").as_double();
        double momentum_weight = this->get_parameter("momentum_weight").as_double();
        double linear_matching_weight = this->get_parameter("linear_matching_weight").as_double();
        double twist_matching_weight = this->get_parameter("twist_matching_weight").as_double();

        double time_to_collision_threshold = this->get_parameter("time_to_collision_threshold").as_double();
        double radius_threshold = this->get_parameter("radius_threshold").as_double();
        
        std::string desired_vel_topic = this->get_parameter("topics.desired_vel").as_string();
        std::string cmd_vel_topic = this->get_parameter("topics.cmd_vel").as_string();
        std::string odometry_topic = this->get_parameter("topics.odometry").as_string();
        std::string element_tracking_topic = this->get_parameter("topics.element_tracking").as_string();
        std::string goal_odometry_topic = this->get_parameter("topics.goal_odometry").as_string();
        
        std::cout << "================================================================" << std::endl;
        std::cout << "================== S3VO NODE INITIALIZATION =================" << std::endl;
        std::cout << "================================================================" << std::endl;
        
        std::cout << "Frame Settings:" << std::endl;
        std::cout << "  - Fixed Frame: " << fixed_frame << std::endl;
        std::cout << "  - Agent Frame: " << agent_frame << std::endl;
        
        std::cout << "Using topics:" << std::endl;
        std::cout << "  - Cmd Velocity: " << cmd_vel_topic << std::endl;
        std::cout << "  - Desired Velocity: " << desired_vel_topic << std::endl;
        std::cout << "  - Odometry: " << odometry_topic << std::endl;
        std::cout << "  - Element Tracking: " << element_tracking_topic << std::endl;
        std::cout << "  - Goal Odometry: " << goal_odometry_topic << std::endl;
        
        std::cout << "S3VO Parameters:" << std::endl;
        std::cout << "  - Kinematic Mode: " << kinematic_mode << std::endl;
        std::cout << "  - Vehicle Radius: " << std::fixed << std::setprecision(2) << vehicle_radius << std::endl;
        std::cout << "  - Heading Weight: " << heading_weight << std::endl;
        std::cout << "  - Danger Weight: " << danger_weight << std::endl;
        std::cout << "  - Absolute Weight: " << abs_weight << std::endl;
        std::cout << "  - Momentum Weight: " << momentum_weight << std::endl;
        std::cout << "  - Linear Matching Weight: " << linear_matching_weight << std::endl;
        std::cout << "  - Twist Matching Weight: " << twist_matching_weight << std::endl;
        
        std::cout << "Motion Limits:" << std::endl;
        std::cout << "  - Max Linear Velocity [X,Y,Z]: ["
                  << std::fixed << std::setprecision(2)
                  << this->get_parameter("v_x_max").as_double() << ", "
                  << this->get_parameter("v_y_max").as_double() << ", "
                  << this->get_parameter("v_z_max").as_double() << "]" << std::endl;
        
        std::cout << "  - Max Angular Velocity [R,P,Y]: ["
                  << this->get_parameter("w_roll_max").as_double() << ", "
                  << this->get_parameter("w_pitch_max").as_double() << ", "
                  << this->get_parameter("w_yaw_max").as_double() << "]" << std::endl;
        
        std::cout << "================================================================\n" << std::endl;

        // Initialize the appropriate sample generator based on kinematic mode
        if (kinematic_mode == "diff_drive") {
            std::cout << "Using differential drive sample generator" << std::endl;
            sample_generator_ = std::make_unique<s3vo::DiffDriveSampleGenerator>(this->get_logger());
        }
        else if (kinematic_mode == "holonomic_ellipsoidal"){
            std::cout << "Using holonomic ellipsoidal sample generator" << std::endl;
            sample_generator_ = std::make_unique<s3vo::HolonomicEllipsoidalSampleGenerator>(this->get_logger());

        } else {
            // Default to holonomic
            std::cout << "Using holonomic sample generator" << std::endl;
            sample_generator_ = std::make_unique<s3vo::HolonomicSampleGenerator>(this->get_logger());
        }
                

        
        //! Set parameters in the sample generator
        sample_generator_->setParams(
            this->get_parameter("v_x_max").as_double(), 
            this->get_parameter("v_y_max").as_double(), 
            this->get_parameter("v_z_max").as_double(),
            this->get_parameter("a_x_max").as_double(), 
            this->get_parameter("a_y_max").as_double(), 
            this->get_parameter("a_z_max").as_double(),
            this->get_parameter("w_roll_max").as_double(), 
            this->get_parameter("w_pitch_max").as_double(), 
            this->get_parameter("w_yaw_max").as_double(),
            this->get_parameter("a_roll_max").as_double(), 
            this->get_parameter("a_pitch_max").as_double(), 
            this->get_parameter("a_yaw_max").as_double(),
            this->get_parameter("delta_t").as_double(), 
            this->get_parameter("num_samples").as_int());
            


        //! Initialize evaluator and visualizer
        sample_evaluator_ = std::make_unique<s3vo::SampleEvaluator>(
            this->get_logger(), 
            vehicle_radius, 
            heading_weight,
            danger_weight, 
            abs_weight,    
            momentum_weight,
            linear_matching_weight,
            twist_matching_weight,
            time_to_collision_threshold,  
            radius_threshold);           
    
           
        sample_visualizer_ = std::make_unique<s3vo::SampleVisualizer>(this);

        // Publishers and subscribers - keep these as they are
        cmd_vel_publisher_ = this->create_publisher<geometry_msgs::msg::TwistStamped>(cmd_vel_topic, 10);
        
        desired_velocity_subscriber_ = this->create_subscription<geometry_msgs::msg::TwistStamped>(
            desired_vel_topic, 10, std::bind(&S3VO::desired_velocity_callback, this, std::placeholders::_1));
            
        agent_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
            odometry_topic, 10, std::bind(&S3VO::agent_odometry_callback, this, std::placeholders::_1));
        
        obstacles_subscriber_ = this->create_subscription<s3vo::msg::ElementCharacteristicsArray>(
            element_tracking_topic, 10, std::bind(&S3VO::obstacles_callback, this, std::placeholders::_1));
        
        goal_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
            goal_odometry_topic, 10, std::bind(&S3VO::goal_callback, this, std::placeholders::_1));

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(static_cast<int>(100)), 
            std::bind(&S3VO::timer_callback, this));
        
        last_odom_time_ = this->get_clock()->now();
        last_desired_vel_time_ = this->get_clock()->now();
    }

private:
    // Callback functions
    void desired_velocity_callback(const geometry_msgs::msg::TwistStamped::SharedPtr msg)
    {
        latest_desired_velocity_ = msg->twist;
        sample_evaluator_->setDesiredVelocity(msg->twist);
        last_desired_vel_time_ = this->get_clock()->now();
        RCLCPP_DEBUG(this->get_logger(), "Updated agent desired velocity");
    }

    void goal_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        latest_goal_odometry_ = *msg;
        RCLCPP_DEBUG(this->get_logger(), "Updated goal odometry");
    }

    void agent_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        latest_agent_odometry_ = *msg;
        sample_visualizer_->setAgentOdometry(*msg);
        last_odom_time_ = this->get_clock()->now();
        RCLCPP_DEBUG(this->get_logger(), "Updated agent odometry");
    }

    void obstacles_callback(const s3vo::msg::ElementCharacteristicsArray::SharedPtr msg)
    {
        latest_obstacles_ = *msg;
        RCLCPP_DEBUG(this->get_logger(), "Updated obstacles");
    }
    
    void timer_callback()
    {
        // Timer callback to eval time
        
        geometry_msgs::msg::TwistStamped cmd_vel;
        
        // --- WATCHDOG CHECK ---
        auto now = this->get_clock()->now();
        double timeout = this->get_parameter("watchdog_timeout").as_double();
        bool odom_timeout = (now - last_odom_time_).seconds() > timeout;
        bool desired_vel_timeout = (now - last_desired_vel_time_).seconds() > timeout;

        if (odom_timeout || desired_vel_timeout) {
            if (odom_timeout) RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000, "WATCHDOG: Odometry timeout! Forcing zero velocity.");
            if (desired_vel_timeout) RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000, "WATCHDOG: Desired Velocity timeout! Forcing zero velocity.");
            
            cmd_vel.header.stamp = now;
            cmd_vel.header.frame_id = this->get_parameter("fixed_frame").as_string();
            cmd_vel.twist = geometry_msgs::msg::Twist(); // Zero velocity
            cmd_vel_publisher_->publish(cmd_vel);
            return;
        }

        if (!has_received_all_data()) {
            RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                "Waiting for Odometry and Desired Velocity data...");
            return;
        }

        geometry_msgs::msg::Twist current_velocity = latest_agent_odometry_.twist.twist;
        samples = sample_generator_->generateSamples(current_velocity, latest_desired_velocity_);
        sample_evaluator_->evaluateSamples(samples, latest_obstacles_, current_velocity);

        best_sample = sample_evaluator_->findBestSample(samples);
        
        // Directly use the twist stored in the best sample (No more translation needed!)
        best_twist.linear.x = best_sample.twist.lx;
        best_twist.linear.y = best_sample.twist.ly;
        best_twist.linear.z = best_sample.twist.lz;
        best_twist.angular.x = best_sample.twist.ax;
        best_twist.angular.y = best_sample.twist.ay;
        best_twist.angular.z = best_sample.twist.az;

        cmd_vel.header.stamp = this->get_clock()->now();
        cmd_vel.header.frame_id = this->get_parameter("fixed_frame").as_string();
        cmd_vel.twist = best_twist;

        cmd_vel_publisher_->publish(cmd_vel);
        sample_visualizer_->publishSamplesAsPointcloud(samples, best_sample);

    }
    
    bool has_received_all_data()
    {
        // Check if we've received all necessary data
        static bool have_agent_data = false;
        static bool have_desired_velocity = false;
        
        if (!have_agent_data && latest_agent_odometry_.header.stamp.sec != 0) {
            have_agent_data = true;
        }
        
        if (!have_desired_velocity && 
           (std::abs(latest_desired_velocity_.linear.x) > 0.001 || 
            std::abs(latest_desired_velocity_.linear.y) > 0.001 || 
            std::abs(latest_desired_velocity_.linear.z) > 0.001 ||
            std::abs(latest_desired_velocity_.angular.x) > 0.001 ||
            std::abs(latest_desired_velocity_.angular.y) > 0.001 ||
            std::abs(latest_desired_velocity_.angular.z) > 0.001)) {
            have_desired_velocity = true;
        }
        
        return have_agent_data && have_desired_velocity;
    }

    
    // State variables and components
    std::unique_ptr<s3vo::SampleGenerator> sample_generator_;
    std::unique_ptr<s3vo::SampleEvaluator> sample_evaluator_;
    std::unique_ptr<s3vo::SampleVisualizer> sample_visualizer_;
    std::vector<VelocitySample> samples;
    VelocitySample best_sample;
    geometry_msgs::msg::Twist best_twist;

    // Publishers and subscribers
    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr cmd_vel_publisher_;
    rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr desired_velocity_subscriber_;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr agent_odometry_subscriber_;
    rclcpp::Subscription<s3vo::msg::ElementCharacteristicsArray>::SharedPtr obstacles_subscriber_;
    //goal odometry
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr goal_odometry_subscriber_;
    rclcpp::TimerBase::SharedPtr timer_;
    

    // Message storage
    nav_msgs::msg::Odometry latest_goal_odometry_{};
    geometry_msgs::msg::Twist latest_desired_velocity_{};
    nav_msgs::msg::Odometry latest_agent_odometry_{};
    s3vo::msg::ElementCharacteristicsArray latest_obstacles_{};

    // Watchdog timing
    rclcpp::Time last_odom_time_;
    rclcpp::Time last_desired_vel_time_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<S3VO>());
    rclcpp::shutdown();
    return 0;
}