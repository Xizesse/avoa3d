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


using namespace std::chrono_literals;
using avoa3d::VelocitySample;


class AVOA : public rclcpp::Node
{
public:
    AVOA() : Node("avoa3dnode", rclcpp::NodeOptions())
    //AVOA() : Node("avoa3dnode", rclcpp::NodeOptions().allow_undeclared_parameters(true).automatically_declare_parameters_from_overrides(true))
    
    {
        kinematic_mode = this->declare_parameter<std::string>("kinematic_mode", "unknown");

        RCLCPP_INFO(this->get_logger(), "Initializing AVOA node with kinematic mode: %s", kinematic_mode.c_str());


        //sample_generator_ = std::make_unique<avoa3d::DiffDriveSampleGenerator>(this->get_logger());
        //sample_generator_ = std::make_unique<avoa3d::HolonomicSampleGenerator>(this->get_logger());
        
        //! Change generator according to kinematic mode
        if (kinematic_mode == "diff_drive") {
            RCLCPP_INFO(this->get_logger(), "Using differential drive sample generator");
            sample_generator_ = std::make_unique<avoa3d::DiffDriveSampleGenerator>(
                this->get_logger(), this);
        } else {
            // Default to holonomic
            RCLCPP_INFO(this->get_logger(), "Using holonomic sample generator");
            sample_generator_ = std::make_unique<avoa3d::HolonomicSampleGenerator>(
                this->get_logger(), this);
        }

        sample_evaluator_ = std::make_unique<avoa3d::SampleEvaluator>(this->get_logger(), vehicle_radius_);
        sample_visualizer_ = std::make_unique<avoa3d::SampleVisualizer>(this);


        //! PUBLISHERS
        cmd_vel_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/model/agente/cmd_vel", 10);
        
        //! SUBSCRIBERS
        desired_velocity_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/model/agente/desired_vel", 10, std::bind(&AVOA::desired_velocity_callback, this, std::placeholders::_1));
            
        velocity_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/model/agente/cmd_vel", 10, std::bind(&AVOA::velocity_callback, this, std::placeholders::_1));
            
        agent_odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
            "/model/agente/odometry", 10, std::bind(&AVOA::agent_odometry_callback, this, std::placeholders::_1));
        
        obstacles_subscriber_ = this->create_subscription<custom_msgs::msg::ElementCharacteristicsArray>(
            "/element_tracking/elements", 10, std::bind(&AVOA::obstacles_callback, this, std::placeholders::_1));

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(static_cast<int>(200)), 
            std::bind(&AVOA::timer_callback, this));
            
    }

private:
    
    //std::unique_ptr<avoa3d::DiffDriveSampleGenerator> sample_generator_;
    std::unique_ptr<avoa3d::SampleGenerator> sample_generator_;//! Generator (virtual)
    std::unique_ptr<avoa3d::SampleEvaluator> sample_evaluator_;//! Evaluator
    std::unique_ptr<avoa3d::SampleVisualizer> sample_visualizer_;//! Visualizer
    std::vector<VelocitySample> samples;
    VelocitySample best_sample;
    geometry_msgs::msg::Twist best_twist;


    //! Callback functions
    void desired_velocity_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        latest_desired_velocity_ = *msg;
        sample_evaluator_->setDesiredVelocity(*msg);
        RCLCPP_DEBUG(this->get_logger(), "Updated agent desired velocity");
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

        if(latest_desired_velocity_.linear.x == 0.0 && latest_desired_velocity_.linear.y == 0.0 && latest_desired_velocity_.linear.z == 0.0)
        {
            cmd_vel.linear.x = 0.0; cmd_vel.linear.y = 0.0; cmd_vel.linear.z = 0.0;
            cmd_vel.angular.x = 0.0; cmd_vel.angular.y = 0.0; cmd_vel.angular.z = 0.0;
            cmd_vel_publisher_->publish(cmd_vel);
            return;
        }

        samples = sample_generator_->generateSamples(latest_velocity_, latest_desired_velocity_);

        sample_evaluator_->evaluateSamples(samples, latest_obstacles_);
        
        best_sample = sample_evaluator_->findBestSample(samples);
        
        best_twist = sample_generator_->translateToTwist(best_sample);

        //!BASIC FILTERING 0.8 current + 0.2 new   
        //cmd_vel.linear.x = 0.8*latest_velocity_.linear.x + 0.2*best_sample.vx + 0.0*latest_desired_velocity_.linear.x;
        //cmd_vel.linear.y = 0.8*latest_velocity_.linear.y + 0.2*best_sample.vy + 0.0*latest_desired_velocity_.linear.y;
        //cmd_vel.linear.z = 0.8*latest_velocity_.linear.z + 0.2*best_sample.vz + 0.0*latest_desired_velocity_.linear.z;

        //cmd_vel.angular.x = 0.0;
        //cmd_vel.angular.y = 0.0;
        //cmd_vel.angular.z = 0.0;

        cmd_vel.linear.x = 0.5*best_twist.linear.x + 0.5*latest_velocity_.linear.x;
        cmd_vel.linear.y = 0.5*best_twist.linear.y + 0.5*latest_velocity_.linear.y;
        cmd_vel.linear.z = 0.5*best_twist.linear.z + 0.5*latest_velocity_.linear.z;

        cmd_vel.angular.x = 0.5*best_twist.angular.x + 0.5*latest_velocity_.angular.x;
        cmd_vel.angular.y = 0.5*best_twist.angular.y + 0.5*latest_velocity_.angular.y;
        cmd_vel.angular.z = 0.5*best_twist.angular.z + 0.5*latest_velocity_.angular.z;

        
        /* RCLCPP_INFO(this->get_logger(), "Best Sample: vx: %.2f, vy: %.2f, vz: %.2f, cost: %.2f, danger: %.2f", 
            best_sample.vx, best_sample.vy, best_sample.vz, best_sample.cost, best_sample.danger);
        RCLCPP_INFO(this->get_logger(), "Best Twist: vx: %.2f, vy: %.2f, vz: %.2f, ax: %.2f, ay: %.2f, az: %.2f", 
            best_twist.linear.x, best_twist.linear.y, best_twist.linear.z, best_twist.angular.x, best_twist.angular.y, best_twist.angular.z);
 */
        /* cmd_vel.linear.x = best_sample.vx;
        cmd_vel.linear.y = best_sample.vy;
        cmd_vel.linear.z = best_sample.vz;
        cmd_vel.angular.x = 0.0;
        cmd_vel.angular.y = 0.0;
        cmd_vel.angular.z = 0.0; */

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
            //RCLCPP_INFO(this->get_logger(), "Received initial agent odometry data");
        }
        
        if (!have_desired_velocity && 
           (std::abs(latest_desired_velocity_.linear.x) > 0.001 || 
            std::abs(latest_desired_velocity_.linear.y) > 0.001 || 
            std::abs(latest_desired_velocity_.linear.z) > 0.001)) {
            have_desired_velocity = true;
            //RCLCPP_INFO(this->get_logger(), "Received initial desired velocity data");
        }
        
        return have_agent_data && have_desired_velocity;
    }
    
    
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_publisher_;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr samples_cloud_publisher_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr desired_velocity_subscriber_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr velocity_subscriber_;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr agent_odometry_subscriber_;
    rclcpp::Subscription<custom_msgs::msg::ElementCharacteristicsArray>::SharedPtr obstacles_subscriber_;
    rclcpp::TimerBase::SharedPtr timer_;
    
    // State variables
    geometry_msgs::msg::Twist latest_desired_velocity_{};
    geometry_msgs::msg::Twist latest_velocity_{};
    nav_msgs::msg::Odometry latest_agent_odometry_{};
    custom_msgs::msg::ElementCharacteristicsArray latest_obstacles_{};
    
    // Parameters
    std::string kinematic_mode;
    double vehicle_radius_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AVOA>());
    rclcpp::shutdown();
    return 0;
}