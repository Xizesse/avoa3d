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


using namespace std::chrono_literals;
using avoa3d::VelocitySample;


class AVOA : public rclcpp::Node
{
public:
    AVOA() : Node("avoa3dnode")
    {
        declare_parameters();
        
        sample_evaluator_ = std::make_unique<avoa3d::SampleEvaluator>(this->get_logger(), vehicle_radius_);
        sample_visualizer_ = std::make_unique<avoa3d::SampleVisualizer>(this, delta_t_);


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
            std::chrono::milliseconds(static_cast<int>(100)), 
            std::bind(&AVOA::timer_callback, this));
            
        RCLCPP_INFO(this->get_logger(), "AVOA node initialized with parameters:");
        RCLCPP_INFO(this->get_logger(), "  v_x_max: %.2f", v_x_max_);
        RCLCPP_INFO(this->get_logger(), "  v_y_max: %.2f", v_y_max_);
        RCLCPP_INFO(this->get_logger(), "  v_z_max: %.2f", v_z_max_);
        RCLCPP_INFO(this->get_logger(), "  delta_t: %.4f", delta_t_);
        RCLCPP_INFO(this->get_logger(), "  num_samples: %d", num_samples_);
    }

private:
    
    std::unique_ptr<avoa3d::SampleEvaluator> sample_evaluator_;//! Evaluator
    std::unique_ptr<avoa3d::SampleVisualizer> sample_visualizer_;//! Visualizer
    std::vector<VelocitySample> samples;
    VelocitySample best_sample;

    // TODO Parameter declaration (Hardcoded rn)
    void declare_parameters()
    {
        // Maximum accelerations
        a_x_max_ = 0.5;
        a_y_max_ = 0.5;
        a_z_max_ = 0.5;
        
        // Maximum velocities
        v_x_max_ = 0.5;
        v_y_max_ = 0.5;
        v_z_max_ = 0.0;
    
        // Time step
        delta_t_ = 5.0;
        
        // Vehicle radius
        vehicle_radius_ = 0.5;

        // Sampling parameters
        num_samples_ = 2000;
        
        // Optional weights
        double heading_weight = 0.7;
        double magnitude_error_weight = 0.3;
        double safety_weight = 0.5;
        
        RCLCPP_INFO(this->get_logger(), "Using hardcoded parameters:");
        RCLCPP_INFO(this->get_logger(), "  a_x_max: %.2f", a_x_max_);
        RCLCPP_INFO(this->get_logger(), "  a_y_max: %.2f", a_y_max_);
        RCLCPP_INFO(this->get_logger(), "  a_z_max: %.2f", a_z_max_);
        RCLCPP_INFO(this->get_logger(), "  v_x_max: %.2f", v_x_max_);
        RCLCPP_INFO(this->get_logger(), "  v_y_max: %.2f", v_y_max_);
        RCLCPP_INFO(this->get_logger(), "  v_z_max: %.2f", v_z_max_);
        RCLCPP_INFO(this->get_logger(), "  delta_t: %.2f", delta_t_);
        RCLCPP_INFO(this->get_logger(), "  vehicle_radius: %.2f", vehicle_radius_);
        RCLCPP_INFO(this->get_logger(), "  num_samples: %d", num_samples_);
    }
    
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
        if (!has_received_all_data()) {
            return;
        }
        
        samples = generate_velocity_samples();
        
        sample_evaluator_->evaluateSamples(samples, latest_obstacles_, delta_t_);
        
        best_sample = sample_evaluator_->findBestSample(samples);
        
        geometry_msgs::msg::Twist cmd_vel;
        cmd_vel.linear.x = best_sample.vx;
        cmd_vel.linear.y = best_sample.vy;
        cmd_vel.linear.z = best_sample.vz;
        cmd_vel.angular.x = 0.0;
        cmd_vel.angular.y = 0.0;
        cmd_vel.angular.z = 0.0;
        
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
    
    std::vector<VelocitySample> generate_velocity_samples()
    {
        std::vector<VelocitySample> samples;
        std::random_device rd;
        // random number generation
        std::mt19937 gen(rd());
        
        // Calculate velocity limits based on current velocity and acceleration constraints
        // Lower bounds: current velocity - max acceleration * delta_t (but not below -v_max)
        // Upper bounds: current velocity + max acceleration * delta_t (but not above v_max)
        double min_vx = std::max(latest_velocity_.linear.x - a_x_max_ * delta_t_, -v_x_max_);
        double max_vx = std::min(latest_velocity_.linear.x + a_x_max_ * delta_t_, v_x_max_);
        
        double min_vy = std::max(latest_velocity_.linear.y - a_y_max_ * delta_t_, -v_y_max_);
        double max_vy = std::min(latest_velocity_.linear.y + a_y_max_ * delta_t_, v_y_max_);
        
        double min_vz = std::max(latest_velocity_.linear.z - a_z_max_ * delta_t_, -v_z_max_);
        double max_vz = std::min(latest_velocity_.linear.z + a_z_max_ * delta_t_, v_z_max_);
    
        // Create distributions based on the calculated bounds
        std::uniform_real_distribution<> dist_vx(min_vx, max_vx);
        std::uniform_real_distribution<> dist_vy(min_vy, max_vy);
        std::uniform_real_distribution<> dist_vz(min_vz, max_vz);
        
        // Generate random samples
        for (int i = 0; i < num_samples_; ++i) {
            double vx = dist_vx(gen);
            double vy = dist_vy(gen);
            double vz = dist_vz(gen);
            
            samples.push_back(VelocitySample(vx, vy, vz));
        }
        
        // Always include the current velocity as a sample,//TODO but chek if min and max values are okay
        samples.push_back(VelocitySample(
            latest_velocity_.linear.x,
            latest_velocity_.linear.y,
            latest_velocity_.linear.z
        ));
        
        // If the desired velocity is within the reachable range, include it
        double desired_vx = latest_desired_velocity_.linear.x;
        double desired_vy = latest_desired_velocity_.linear.y;
        double desired_vz = latest_desired_velocity_.linear.z;
        
        bool desired_velocity_reachable = 
            (std::abs(desired_vx) > 0.001 || std::abs(desired_vy) > 0.001 || std::abs(desired_vz) > 0.001) &&
            (desired_vx >= min_vx && desired_vx <= max_vx) &&
            (desired_vy >= min_vy && desired_vy <= max_vy) &&
            (desired_vz >= min_vz && desired_vz <= max_vz);

        if (desired_velocity_reachable) {
            samples.push_back(VelocitySample(desired_vx, desired_vy, desired_vz));
        }
        
        return samples;
    }
    

    VelocitySample find_best_sample(const std::vector<VelocitySample>& samples)
    {
        auto min_element = std::min_element(
            samples.begin(), samples.end(),
            [](const VelocitySample& a, const VelocitySample& b) { return a.cost < b.cost; }
        );
        
        return *min_element;
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
    double a_x_max_;
    double a_y_max_;
    double a_z_max_;
    double v_x_max_;
    double v_y_max_;
    double v_z_max_;
    double delta_t_;
    int num_samples_;
    double vehicle_radius_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AVOA>());
    rclcpp::shutdown();
    return 0;
}