#include <chrono>
#include <memory>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/point_cloud2_iterator.hpp"
#include "custom_msgs/msg/element_characteristics_array.hpp"
#include "avoa3d/velocity_sample.hpp"
#include "avoa3d/sample_evaluator.hpp"
#include "avoa3d/sample_visualizer.hpp"
#include "avoa3d/sample_generator.hpp"
#include "avoa3d/motion_params.hpp"

using namespace std::chrono_literals;
using avoa3d::VelocitySample;
using avoa3d::MotionParameters;

class AVOA : public rclcpp::Node
{
public:
    AVOA() : Node("avoa3dnode", rclcpp::NodeOptions())
    {
        // Declare frame ID parameters with defaults
        this->declare_parameter<std::string>("fixed_frame", "map");
        this->declare_parameter<std::string>("agent_frame", "agent");
        
        fixed_frame_ = this->get_parameter("fixed_frame").as_string();
        agent_frame_ = this->get_parameter("agent_frame").as_string();
        
        RCLCPP_INFO(this->get_logger(), "Using frames: fixed=%s, agent=%s", 
                    fixed_frame_.c_str(), agent_frame_.c_str());

        // Declare and load all parameters
        loadParameters();
        
        RCLCPP_INFO(this->get_logger(), "Initializing AVOA node with kinematic mode: %s", motion_params_.kinematic_mode.c_str());
        RCLCPP_INFO(this->get_logger(), "Vehicle radius: %.2f", motion_params_.vehicle_radius);

        // Initialize the appropriate sample generator
        if (motion_params_.kinematic_mode == "diff_drive") {
            RCLCPP_INFO(this->get_logger(), "Using differential drive sample generator");
            sample_generator_ = std::make_unique<avoa3d::DiffDriveSampleGenerator>(
                this->get_logger(), this);
        } else {
            // Default to holonomic
            RCLCPP_INFO(this->get_logger(), "Using holonomic sample generator");
            sample_generator_ = std::make_unique<avoa3d::HolonomicSampleGenerator>(
                this->get_logger(), this);
        }
        
        // Set parameters in the sample generator
        sample_generator_->setMotionParameters(motion_params_);

        // Initialize evaluator and visualizer
        sample_evaluator_ = std::make_unique<avoa3d::SampleEvaluator>(
            this->get_logger(), motion_params_.vehicle_radius, motion_params_.heading_weight,
            motion_params_.danger_weight, motion_params_.abs_weight);
        sample_visualizer_ = std::make_unique<avoa3d::SampleVisualizer>(this);

        // Publishers and subscribers
        cmd_vel_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/avoa/cmd_vel", 10);
        
        desired_velocity_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/model/agente/desired_vel", 10, std::bind(&AVOA::desired_velocity_callback, this, std::placeholders::_1));
            
        velocity_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
           "/avoa/cmd_vel", 10, std::bind(&AVOA::velocity_callback, this, std::placeholders::_1));
        //velocity_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
        //    "/nest/cmd_vel", 10, std::bind(&AVOA::velocity_callback, this, std::placeholders::_1));    
        
        agent_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
            "/nest/odometry", 10, std::bind(&AVOA::agent_odometry_callback, this, std::placeholders::_1));
        //agent_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
            //"/nest/odometry", 10, std::bind(&AVOA::agent_odometry_callback, this, std::placeholders::_1));

        obstacles_subscriber_ = this->create_subscription<custom_msgs::msg::ElementCharacteristicsArray>(
            "/element_tracking/elements", 10, std::bind(&AVOA::obstacles_callback, this, std::placeholders::_1));

        //!Not being used rn
        goal_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
            "/model/goal/odometry", 10, std::bind(&AVOA::goal_callback, this, std::placeholders::_1));

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(static_cast<int>(200)), 
            std::bind(&AVOA::timer_callback, this));
    }

private:
    std::string fixed_frame_;
    std::string agent_frame_;

    // Load all parameters from ROS Parameter Server
    void loadParameters() {
        // AVOA Parameters
        motion_params_.kinematic_mode = this->declare_parameter<std::string>("kinematic_mode", "holonomic");
        motion_params_.vehicle_radius = this->declare_parameter<double>("vehicle_radius", 0.5);
        motion_params_.heading_weight = this->declare_parameter<double>("heading_weight", 0.5);
        motion_params_.abs_weight = this->declare_parameter<double>("abs_weight", 0.5);
        motion_params_.danger_weight = this->declare_parameter<double>("danger_weight", 0.5);
        
        // Motion Parameters
        motion_params_.a_x_max = this->declare_parameter<double>("a_x_max", 3.0);
        motion_params_.a_y_max = this->declare_parameter<double>("a_y_max", 3.0);
        motion_params_.a_z_max = this->declare_parameter<double>("a_z_max", 3.0);
        
        motion_params_.a_roll_max = this->declare_parameter<double>("a_roll_max", 0.0);
        motion_params_.a_pitch_max = this->declare_parameter<double>("a_pitch_max", 0.0);
        motion_params_.a_yaw_max = this->declare_parameter<double>("a_yaw_max", 0.0);
        
        motion_params_.v_x_max = this->declare_parameter<double>("v_x_max", 1.0);
        motion_params_.v_y_max = this->declare_parameter<double>("v_y_max", 1.0);
        motion_params_.v_z_max = this->declare_parameter<double>("v_z_max", 0.0);
        
        motion_params_.w_roll_max = this->declare_parameter<double>("w_roll_max", 0.0);
        motion_params_.w_pitch_max = this->declare_parameter<double>("w_pitch_max", 0.0);
        motion_params_.w_yaw_max = this->declare_parameter<double>("w_yaw_max", 0.0);
        
        motion_params_.delta_t = this->declare_parameter<double>("delta_t", 1.0);
        motion_params_.num_samples = this->declare_parameter<int>("num_samples", 10000);
        motion_params_.filtering_obstacles = this->declare_parameter<bool>("filtering_obstacles", false);
    }
    
    // Callback functions
    void desired_velocity_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        latest_desired_velocity_ = *msg;
        sample_evaluator_->setDesiredVelocity(*msg);
        RCLCPP_DEBUG(this->get_logger(), "Updated agent desired velocity");
    }

    void goal_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        latest_goal_odometry_ = *msg;
        RCLCPP_DEBUG(this->get_logger(), "Updated goal odometry");
    }

    void velocity_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        latest_velocity_ = *msg;
        RCLCPP_DEBUG(this->get_logger(), "Updated agent velocity");
    }
    
    void agent_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        latest_agent_odometry_ = *msg;
        sample_visualizer_->setAgentOdometry(*msg);
        RCLCPP_DEBUG(this->get_logger(), "Updated agent odometry");
    }

    void obstacles_callback(const custom_msgs::msg::ElementCharacteristicsArray::SharedPtr msg)
    {
        latest_obstacles_ = *msg;
        RCLCPP_DEBUG(this->get_logger(), "Updated obstacles");
    }
    
    void timer_callback()
    {
        geometry_msgs::msg::Twist cmd_vel;
        if (!has_received_all_data()) {
            return;
        }

        /*
         if(latest_desired_velocity_.linear.x == 0.0 && 
           latest_desired_velocity_.linear.y == 0.0 && 
           latest_desired_velocity_.linear.z == 0.0)
        {
            cmd_vel.linear.x = 0.0; cmd_vel.linear.y = 0.0; cmd_vel.linear.z = 0.0;
            cmd_vel.angular.x = 0.0; cmd_vel.angular.y = 0.0; cmd_vel.angular.z = 0.0;
            cmd_vel_publisher_->publish(cmd_vel);
            return;
        } */
        
       /*
       calc goal distance
        create a new obstacle array message
        for each obstacle in the old obstacle array
        check distance
        if distance to the goal is greater than distance to the obstacles
        add then to the new messas
        */
        
        if (motion_params_.filtering_obstacles)
        {
            float distance_to_goal = std::sqrt(
                std::pow(latest_goal_odometry_.pose.pose.position.x - latest_agent_odometry_.pose.pose.position.x, 2) +
                std::pow(latest_goal_odometry_.pose.pose.position.y - latest_agent_odometry_.pose.pose.position.y, 2) +
                std::pow(latest_goal_odometry_.pose.pose.position.z - latest_agent_odometry_.pose.pose.position.z, 2)
            );
            custom_msgs::msg::ElementCharacteristicsArray filtered_obstacles;
            for (const auto& obstacle : latest_obstacles_.elements) {
                float distance_to_obstacle = std::sqrt(
                    std::pow(obstacle.pose.position.x - latest_agent_odometry_.pose.pose.position.x, 2) +
                    std::pow(obstacle.pose.position.y - latest_agent_odometry_.pose.pose.position.y, 2) +
                    std::pow(obstacle.pose.position.z - latest_agent_odometry_.pose.pose.position.z, 2)
                );
                if (distance_to_goal + 2 > distance_to_obstacle ) {
                    filtered_obstacles.elements.push_back(obstacle);
                }
            }
            latest_obstacles_ = filtered_obstacles;
        }   

        samples = sample_generator_->generateSamples(latest_velocity_, latest_desired_velocity_);
        sample_evaluator_->evaluateSamples(samples, latest_obstacles_);
        best_sample = sample_evaluator_->findBestSample(samples);
        best_twist = sample_generator_->translateToTwist(best_sample);

        // Apply filtering (weighted average with current velocity)
        cmd_vel.linear.x = 0.3*best_twist.linear.x + 0.7*latest_velocity_.linear.x;
        cmd_vel.linear.y = 0.3*best_twist.linear.y + 0.7*latest_velocity_.linear.y;
        cmd_vel.linear.z = 0.3*best_twist.linear.z + 0.7*latest_velocity_.linear.z;

        cmd_vel.angular.x = 0.5*best_twist.angular.x + 0.5*latest_velocity_.angular.x;
        cmd_vel.angular.y = 0.5*best_twist.angular.y + 0.5*latest_velocity_.angular.y;
        cmd_vel.angular.z = 0.5*best_twist.angular.z + 0.5*latest_velocity_.angular.z;

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
            std::abs(latest_desired_velocity_.linear.z) > 0.001)) {
            have_desired_velocity = true;
        }
        
        return have_agent_data && have_desired_velocity;
    }
    
    // State variables and components
    MotionParameters motion_params_;
    std::unique_ptr<avoa3d::SampleGenerator> sample_generator_;
    std::unique_ptr<avoa3d::SampleEvaluator> sample_evaluator_;
    std::unique_ptr<avoa3d::SampleVisualizer> sample_visualizer_;
    std::vector<VelocitySample> samples;
    VelocitySample best_sample;
    geometry_msgs::msg::Twist best_twist;

    // Publishers and subscribers
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_publisher_;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr samples_cloud_publisher_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr desired_velocity_subscriber_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr velocity_subscriber_;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr agent_odometry_subscriber_;
    rclcpp::Subscription<custom_msgs::msg::ElementCharacteristicsArray>::SharedPtr obstacles_subscriber_;
    //goal odometry
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr goal_odometry_subscriber_;
    rclcpp::TimerBase::SharedPtr timer_;
    

    // Message storage
    nav_msgs::msg::Odometry latest_goal_odometry_{};
    geometry_msgs::msg::Twist latest_desired_velocity_{};
    geometry_msgs::msg::Twist latest_velocity_{};
    nav_msgs::msg::Odometry latest_agent_odometry_{};
    custom_msgs::msg::ElementCharacteristicsArray latest_obstacles_{};
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AVOA>());
    rclcpp::shutdown();
    return 0;
}