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
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/point_cloud2_iterator.hpp"
//#include "custom_msgs/msg/element_characteristics_array.hpp"
#include "avoa3d/msg/element_characteristics_stamped.hpp"
#include "avoa3d/msg/element_characteristics_array.hpp"
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
        
        // Frame parameters
        this->declare_parameter("fixed_frame", "map");
        this->declare_parameter("agent_frame", "agent");
        
        // Topic parameters
        this->declare_parameter("topics.desired_vel", "/model/agente/desired_vel");
        this->declare_parameter("topics.cmd_vel", "/avoa/cmd_vel");
        this->declare_parameter("topics.odometry", "/nest/odometry");
        this->declare_parameter("topics.element_tracking", "/element_tracking/elements");
        this->declare_parameter("topics.goal_odometry", "/model/goal/odometry");
        
        
        // AVOA parameters
        this->declare_parameter("kinematic_mode", "holonomic");
        this->declare_parameter("vehicle_radius", 0.0);
        this->declare_parameter("heading_weight", 0.0);
        this->declare_parameter("abs_weight", 0.0);
        this->declare_parameter("danger_weight", 0.0);
        this->declare_parameter("momentum_weight", 0.1);
        
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
        
        // Log parameters
        std::string fixed_frame = this->get_parameter("fixed_frame").as_string();
        std::string agent_frame = this->get_parameter("agent_frame").as_string();

        std::string kinematic_mode = this->get_parameter("kinematic_mode").as_string();
        
        double vehicle_radius = this->get_parameter("vehicle_radius").as_double();
        double heading_weight = this->get_parameter("heading_weight").as_double();
        double abs_weight = this->get_parameter("abs_weight").as_double();
        double danger_weight = this->get_parameter("danger_weight").as_double();
        double momentum_weight = this->get_parameter("momentum_weight").as_double();

        double time_to_collision_threshold = this->get_parameter("time_to_collision_threshold").as_double();
        double radius_threshold = this->get_parameter("radius_threshold").as_double();
        
        std::string desired_vel_topic = this->get_parameter("topics.desired_vel").as_string();
        std::string cmd_vel_topic = this->get_parameter("topics.cmd_vel").as_string();
        std::string odometry_topic = this->get_parameter("topics.odometry").as_string();
        std::string element_tracking_topic = this->get_parameter("topics.element_tracking").as_string();
        std::string goal_odometry_topic = this->get_parameter("topics.goal_odometry").as_string();
        
        std::cout << "================================================================" << std::endl;
        std::cout << "================== AVOA3D NODE INITIALIZATION =================" << std::endl;
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
        
        std::cout << "AVOA Parameters:" << std::endl;
        std::cout << "  - Kinematic Mode: " << kinematic_mode << std::endl;
        std::cout << "  - Vehicle Radius: " << std::fixed << std::setprecision(2) << vehicle_radius << std::endl;
        std::cout << "  - Heading Weight: " << heading_weight << std::endl;
        std::cout << "  - Danger Weight: " << danger_weight << std::endl;
        std::cout << "  - Absolute Weight: " << abs_weight << std::endl;
        std::cout << "  - Momentum Weight: " << momentum_weight << std::endl;
        
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
            sample_generator_ = std::make_unique<avoa3d::DiffDriveSampleGenerator>(this->get_logger());
        }
        else if (kinematic_mode == "holonomic_ellipsoidal"){
            std::cout << "Using holonomic ellipsoidal sample generator" << std::endl;
            sample_generator_ = std::make_unique<avoa3d::HolonomicEllipsoidalSampleGenerator>(this->get_logger());

        } else {
            // Default to holonomic
            std::cout << "Using holonomic sample generator" << std::endl;
            sample_generator_ = std::make_unique<avoa3d::HolonomicSampleGenerator>(this->get_logger());
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
        sample_evaluator_ = std::make_unique<avoa3d::SampleEvaluator>(
            this->get_logger(), 
            vehicle_radius, 
            heading_weight,
            danger_weight, 
            abs_weight,    
            momentum_weight,
            time_to_collision_threshold,  
            radius_threshold);           
    
           
        sample_visualizer_ = std::make_unique<avoa3d::SampleVisualizer>(this);

        // Publishers and subscribers - keep these as they are
        cmd_vel_publisher_ = this->create_publisher<geometry_msgs::msg::TwistStamped>(cmd_vel_topic, 10);
        
        desired_velocity_subscriber_ = this->create_subscription<geometry_msgs::msg::TwistStamped>(
            desired_vel_topic, 10, std::bind(&AVOA::desired_velocity_callback, this, std::placeholders::_1));
            
        velocity_subscriber_ = this->create_subscription<geometry_msgs::msg::TwistStamped>(
            cmd_vel_topic, 10, std::bind(&AVOA::velocity_callback, this, std::placeholders::_1));
        
        agent_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
            odometry_topic, 10, std::bind(&AVOA::agent_odometry_callback, this, std::placeholders::_1));
        
        obstacles_subscriber_ = this->create_subscription<avoa3d::msg::ElementCharacteristicsArray>(
            element_tracking_topic, 10, std::bind(&AVOA::obstacles_callback, this, std::placeholders::_1));
        
        goal_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
            goal_odometry_topic, 10, std::bind(&AVOA::goal_callback, this, std::placeholders::_1));

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(static_cast<int>(100)), 
            std::bind(&AVOA::timer_callback, this));
    }

private:
    // Callback functions
    void desired_velocity_callback(const geometry_msgs::msg::TwistStamped::SharedPtr msg)
    {
        latest_desired_velocity_ = msg->twist;
        sample_evaluator_->setDesiredVelocity(msg->twist);
        RCLCPP_DEBUG(this->get_logger(), "Updated agent desired velocity");
    }

    void goal_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        latest_goal_odometry_ = *msg;
        RCLCPP_DEBUG(this->get_logger(), "Updated goal odometry");
    }

    void velocity_callback(const geometry_msgs::msg::TwistStamped::SharedPtr msg)
    {
        latest_velocity_ = msg->twist;
        RCLCPP_DEBUG(this->get_logger(), "Updated agent velocity");
    }
    
    void agent_odometry_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
        latest_agent_odometry_ = *msg;
        sample_visualizer_->setAgentOdometry(*msg);
        RCLCPP_DEBUG(this->get_logger(), "Updated agent odometry");
    }

    void obstacles_callback(const avoa3d::msg::ElementCharacteristicsArray::SharedPtr msg)
    {
        latest_obstacles_ = *msg;
        RCLCPP_DEBUG(this->get_logger(), "Updated obstacles");
    }
    
    void timer_callback()
    {
        // Timer callback to eval time
        
        geometry_msgs::msg::TwistStamped cmd_vel;
        if (!has_received_all_data()) {
            RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                "Waiting for Odometry and Desired Velocity data...");
            return;
        }

        auto t0 = std::chrono::steady_clock::now();
        
        samples = sample_generator_->generateSamples(latest_velocity_, latest_desired_velocity_);
        sample_evaluator_->evaluateSamples(samples, latest_obstacles_, latest_velocity_);

        auto t1 = std::chrono::steady_clock::now();

        best_sample = sample_evaluator_->findBestSample(samples);
        best_twist = sample_generator_->translateToTwist(best_sample);

        cmd_vel.header.stamp = this->get_clock()->now();
        cmd_vel.header.frame_id = this->get_parameter("fixed_frame").as_string();
        cmd_vel.twist.linear.x = best_twist.linear.x;
        cmd_vel.twist.linear.y = best_twist.linear.y;
        cmd_vel.twist.linear.z = best_twist.linear.z;
        cmd_vel.twist.angular.x = best_twist.angular.x;
        cmd_vel.twist.angular.y = best_twist.angular.y;
        cmd_vel.twist.angular.z = best_twist.angular.z;

        cmd_vel_publisher_->publish(cmd_vel);
        sample_visualizer_->publishSamplesAsPointcloud(samples, best_sample);

        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        meas_total_us_ += duration_us;
        meas_count_++;

        // RCLCPP_INFO(this->get_logger(),
        //             "N=%zu  this_call=%ld us  avg=%ld us",
        //             samples.size(),
        //             duration_us,
        //             meas_total_us_ / meas_count_);

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
    MotionParameters motion_params_;
    std::unique_ptr<avoa3d::SampleGenerator> sample_generator_;
    std::unique_ptr<avoa3d::SampleEvaluator> sample_evaluator_;
    std::unique_ptr<avoa3d::SampleVisualizer> sample_visualizer_;
    std::vector<VelocitySample> samples;
    VelocitySample best_sample;
    geometry_msgs::msg::Twist best_twist;

    // Publishers and subscribers
    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr cmd_vel_publisher_;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr samples_cloud_publisher_;
    rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr desired_velocity_subscriber_;
    rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr velocity_subscriber_;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr agent_odometry_subscriber_;
    rclcpp::Subscription<avoa3d::msg::ElementCharacteristicsArray>::SharedPtr obstacles_subscriber_;
    //goal odometry
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr goal_odometry_subscriber_;
    rclcpp::TimerBase::SharedPtr timer_;
    

    // Message storage
    nav_msgs::msg::Odometry latest_goal_odometry_{};
    geometry_msgs::msg::Twist latest_desired_velocity_{};
    geometry_msgs::msg::Twist latest_velocity_{};
    nav_msgs::msg::Odometry latest_agent_odometry_{};
    avoa3d::msg::ElementCharacteristicsArray latest_obstacles_{};

    size_t callback_counter_ = 0;
    int64_t total_duration_us_ = 0;
    size_t meas_count_ = 0;      // how many cycles we measured
    int64_t meas_total_us_ = 0;  // accumulated time in microseconds

        
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AVOA>());
    rclcpp::shutdown();
    return 0;
}