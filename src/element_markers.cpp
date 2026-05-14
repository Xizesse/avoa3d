#include <chrono>
#include <memory>
#include <string>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/marker.hpp"
#include "visualization_msgs/msg/marker_array.hpp"
#include "avoa3d/msg/element_characteristics_array.hpp"

using namespace std::chrono_literals;

class ElementMarkersNode : public rclcpp::Node
{
public:
    ElementMarkersNode() : Node("element_markers")
    {
        this->declare_parameter<std::string>("elements_topic", "/element_tracking/elements");
        this->declare_parameter<std::string>("markers_topic", "/markers/element_array");
        this->declare_parameter<std::string>("frame_id", "base_link");

        std::string elements_topic = this->get_parameter("elements_topic").as_string();
        std::string markers_topic = this->get_parameter("markers_topic").as_string();
        frame_id_ = this->get_parameter("frame_id").as_string();

        marker_pub_ = this->create_publisher<visualization_msgs::msg::MarkerArray>(markers_topic, 10);
        
        elements_sub_ = this->create_subscription<avoa3d::msg::ElementCharacteristicsArray>(
            elements_topic, 10,
            std::bind(&ElementMarkersNode::elementsCallback, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Element Markers Node initialized.");
        RCLCPP_INFO(this->get_logger(), "Subscribed to: %s", elements_topic.c_str());
        RCLCPP_INFO(this->get_logger(), "Publishing to: %s", markers_topic.c_str());
    }

private:
    void elementsCallback(const avoa3d::msg::ElementCharacteristicsArray::SharedPtr msg)
    {
        visualization_msgs::msg::MarkerArray marker_array;

        // Delete previous markers
        visualization_msgs::msg::Marker delete_marker;
        delete_marker.action = visualization_msgs::msg::Marker::DELETEALL;
        marker_array.markers.push_back(delete_marker);

        int id = 1;
        for (const auto& element : msg->elements) {
            visualization_msgs::msg::Marker marker;
            marker.header.frame_id = element.header.frame_id.empty() ? frame_id_ : element.header.frame_id;
            marker.header.stamp = this->now();
            marker.ns = "elements";
            marker.id = id++;
            
            // Use CYLINDER to represent obstacle dimensions better
            marker.type = visualization_msgs::msg::Marker::CYLINDER;
            marker.action = visualization_msgs::msg::Marker::ADD;

            marker.pose = element.pose;

            // Enforce minimum visibility scale
            marker.scale.x = std::max(element.size.x, 0.5);
            marker.scale.y = std::max(element.size.y, 0.5);
            marker.scale.z = std::max(element.size.z, 0.5);

            // Vibrant semi-transparent orange color
            marker.color.r = 1.0f;
            marker.color.g = 0.5f;
            marker.color.b = 0.0f;
            marker.color.a = 0.7f;

            marker.lifetime = rclcpp::Duration(1, 0);

            marker_array.markers.push_back(marker);

            // Protective zone visualization (slightly larger and more transparent)
            if (element.protective_zone > 0) {
                visualization_msgs::msg::Marker pz_marker = marker;
                pz_marker.ns = "protective_zones";
                pz_marker.id = id++;
                pz_marker.scale.x += element.protective_zone * 2.0;
                pz_marker.scale.y += element.protective_zone * 2.0;
                // Make protective zone red and very transparent
                pz_marker.color.r = 1.0f;
                pz_marker.color.g = 0.0f;
                pz_marker.color.b = 0.0f;
                pz_marker.color.a = 0.2f;
                marker_array.markers.push_back(pz_marker);
            }
        }

        marker_pub_->publish(marker_array);
    }

    std::string frame_id_;
    rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr marker_pub_;
    rclcpp::Subscription<avoa3d::msg::ElementCharacteristicsArray>::SharedPtr elements_sub_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ElementMarkersNode>());
    rclcpp::shutdown();
    return 0;
}
