#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <cmath>
#include <yaml-cpp/yaml.h>

#include "rclcpp/rclcpp.hpp"
#include "avoa3d/msg/element_characteristics_stamped.hpp"
#include "avoa3d/msg/element_characteristics_array.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "geographic_msgs/msg/geo_point_stamped.hpp"
#include "geometry_msgs/msg/point.hpp"
#include "geometry_msgs/msg/quaternion.hpp"

using namespace std::chrono_literals;

struct StaticObstacle {
    std::string name;
    double lat;
    double lon;
    double radius;
    double height;
    double protective_zone;
    double enu_x = 0.0;
    double enu_y = 0.0;
};

class DummyObstaclePublisher : public rclcpp::Node
{
public:
    DummyObstaclePublisher() : Node("dummy_obstacle_publisher")
    {
        this->declare_parameter<std::string>("agent_frame", "base_link");
        this->declare_parameter<std::string>("odometry_topic", "/usv/lily/mavros/local_position/odom");
        this->declare_parameter<std::string>("origin_topic", "/usv/lily/mavros/global_position/gp_origin");
        this->declare_parameter<double>("publish_rate", 10.0);
        this->declare_parameter<std::string>("config_file", "/home/motusxico/ros2_ws/src/avoa3d/config/static_obstacles.yaml");

        agent_frame_ = this->get_parameter("agent_frame").as_string();
        odometry_topic_ = this->get_parameter("odometry_topic").as_string();
        origin_topic_ = this->get_parameter("origin_topic").as_string();
        publish_rate_ = this->get_parameter("publish_rate").as_double();
        std::string config_file = this->get_parameter("config_file").as_string();

        if (config_file.empty()) {
            RCLCPP_ERROR(this->get_logger(), "No config_file parameter provided!");
        } else {
            loadObstaclesFromYaml(config_file);
        }

        elements_publisher_ = this->create_publisher<avoa3d::msg::ElementCharacteristicsArray>(
            "/element_tracking/elements", 10);

        odometry_subscriber_ = this->create_subscription<nav_msgs::msg::Odometry>(
            odometry_topic_, 10,
            std::bind(&DummyObstaclePublisher::odometryCallback, this, std::placeholders::_1));

        origin_subscriber_ = this->create_subscription<geographic_msgs::msg::GeoPointStamped>(
            origin_topic_, 10,
            std::bind(&DummyObstaclePublisher::originCallback, this, std::placeholders::_1));

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(static_cast<int>(1000.0 / publish_rate_)),
            std::bind(&DummyObstaclePublisher::timerCallback, this));

        RCLCPP_INFO(this->get_logger(), "Static Obstacle Publisher started.");
    }

private:
    void loadObstaclesFromYaml(const std::string& file_path) {
        try {
            YAML::Node config = YAML::LoadFile(file_path);
            if (config["obstacles"]) {
                for (const auto& obs_node : config["obstacles"]) {
                    StaticObstacle obs;
                    obs.name = obs_node["name"].as<std::string>();
                    obs.lat = obs_node["latitude_deg"].as<double>();
                    obs.lon = obs_node["longitude_deg"].as<double>();
                    obs.radius = obs_node["radius"].as<double>();
                    obs.height = obs_node["height"].as<double>();
                    obs.protective_zone = obs_node["protective_zone"].as<double>();
                    obstacles_.push_back(obs);
                    RCLCPP_INFO(this->get_logger(), "Loaded obstacle: %s", obs.name.c_str());
                }
            }
        } catch (const std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "Failed to load YAML: %s", e.what());
        }
    }

    void originCallback(const geographic_msgs::msg::GeoPointStamped::SharedPtr msg) {
        if (origin_received_) return;
        datum_lat_ = msg->position.latitude;
        datum_lon_ = msg->position.longitude;
        origin_received_ = true;

        RCLCPP_INFO(this->get_logger(), "Received origin: lat=%.6f, lon=%.6f", datum_lat_, datum_lon_);

        // Compute ENU coordinates for all static obstacles
        static constexpr double DEG2RAD = M_PI / 180.0;
        static constexpr double SCALE = 111319.5; 
        double lat0_rad = datum_lat_ * DEG2RAD;

        for (auto& obs : obstacles_) {
            obs.enu_x = (obs.lon - datum_lon_) * std::cos(lat0_rad) * SCALE;
            obs.enu_y = (obs.lat - datum_lat_) * SCALE;
            RCLCPP_INFO(this->get_logger(), "Obstacle '%s' at ENU: [%.2f, %.2f]", obs.name.c_str(), obs.enu_x, obs.enu_y);
        }
    }

    void odometryCallback(const nav_msgs::msg::Odometry::SharedPtr msg) {
        latest_odometry_ = *msg;
        has_odometry_ = true;
    }

    geometry_msgs::msg::Point transformToVehicleFrame(
        const geometry_msgs::msg::Point& world_point,
        const geometry_msgs::msg::Point& vehicle_position,
        const geometry_msgs::msg::Quaternion& vehicle_orientation)
    {
        double dx = world_point.x - vehicle_position.x;
        double dy = world_point.y - vehicle_position.y;
        double dz = world_point.z - vehicle_position.z;

        double qw = vehicle_orientation.w;
        double qx = vehicle_orientation.x;
        double qy = vehicle_orientation.y;
        double qz = vehicle_orientation.z;

        double norm = std::sqrt(qw*qw + qx*qx + qy*qy + qz*qz);
        if (norm > 0.0001) {
            qw /= norm; qx /= norm; qy /= norm; qz /= norm;
        }

        double qx_inv = -qx;
        double qy_inv = -qy;
        double qz_inv = -qz;
        double qw_inv = qw;

        double r11 = 1 - 2*(qy_inv*qy_inv + qz_inv*qz_inv);
        double r12 = 2*(qx_inv*qy_inv - qw_inv*qz_inv);
        double r13 = 2*(qx_inv*qz_inv + qw_inv*qy_inv);

        double r21 = 2*(qx_inv*qy_inv + qw_inv*qz_inv);
        double r22 = 1 - 2*(qx_inv*qx_inv + qz_inv*qz_inv);
        double r23 = 2*(qy_inv*qz_inv - qw_inv*qx_inv);

        double r31 = 2*(qx_inv*qz_inv - qw_inv*qy_inv);
        double r32 = 2*(qy_inv*qz_inv + qw_inv*qx_inv);
        double r33 = 1 - 2*(qx_inv*qx_inv + qy_inv*qy_inv);

        geometry_msgs::msg::Point relative_point;
        relative_point.x = r11 * dx + r12 * dy + r13 * dz;
        relative_point.y = r21 * dx + r22 * dy + r23 * dz;
        relative_point.z = r31 * dx + r32 * dy + r33 * dz;

        return relative_point;
    }

    void timerCallback() {
        if (!has_odometry_ || !origin_received_) {
            return;
        }

        auto elements_msg = avoa3d::msg::ElementCharacteristicsArray();
        auto current_time = this->get_clock()->now();

        int id_counter = 1;
        for (const auto& obs : obstacles_) {
            geometry_msgs::msg::Point obs_world;
            obs_world.x = obs.enu_x;
            obs_world.y = obs.enu_y;
            obs_world.z = 0.0;

            geometry_msgs::msg::Point obs_agent = transformToVehicleFrame(
                obs_world,
                latest_odometry_.pose.pose.position,
                latest_odometry_.pose.pose.orientation
            );

            auto element = avoa3d::msg::ElementCharacteristicsStamped();
            element.header.stamp = current_time;
            element.header.frame_id = agent_frame_;
            element.id = id_counter++;
            element.type = 1; // Circular
            element.dynamic = false;

            element.pose.position = obs_agent;
            element.pose.orientation.w = 1.0;
            
            element.velocity.x = 0.0;
            element.velocity.y = 0.0;
            element.velocity.z = 0.0;

            element.size.x = 2.0 * obs.radius;
            element.size.y = 2.0 * obs.radius;
            element.size.z = obs.height;

            element.radius_std = 0.05;
            element.protective_zone = obs.protective_zone;

            elements_msg.elements.push_back(element);
        }

        elements_publisher_->publish(elements_msg);
    }

    std::string agent_frame_;
    std::string odometry_topic_;
    std::string origin_topic_;
    double publish_rate_;

    double datum_lat_;
    double datum_lon_;
    bool origin_received_ = false;

    nav_msgs::msg::Odometry latest_odometry_;
    bool has_odometry_ = false;

    std::vector<StaticObstacle> obstacles_;

    rclcpp::Publisher<avoa3d::msg::ElementCharacteristicsArray>::SharedPtr elements_publisher_;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odometry_subscriber_;
    rclcpp::Subscription<geographic_msgs::msg::GeoPointStamped>::SharedPtr origin_subscriber_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DummyObstaclePublisher>());
    rclcpp::shutdown();
    return 0;
}