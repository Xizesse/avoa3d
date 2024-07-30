#include <chrono>
#include <memory>
#include <vector>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/point_cloud2_iterator.hpp"
#include "geometry_msgs/msg/point.hpp"

float height = 15.0; // m
float width = 30.0; // m
float depth = 30.0; // m
float resolution = 0.2f; // m


geometry_msgs::msg::Point sphere_center;
float sphere_radius = 5.0; // 

using namespace std::chrono_literals;

class PointCloudPublisher : public rclcpp::Node
{
public:
  PointCloudPublisher()
  : Node("point_cloud_publisher")
  {
    sphere_center.x = 0.0;
    sphere_center.y = 0.0;
    sphere_center.z = 0.0;

    publisher_ = this->create_publisher<sensor_msgs::msg::PointCloud2>("point_cloud", 10);
    timer_ = this->create_wall_timer(500ms, std::bind(&PointCloudPublisher::timer_callback, this));
  }

private:
  void timer_callback()
  {
    auto point_cloud_msg = generate_point_cloud();
    publisher_->publish(point_cloud_msg);
    RCLCPP_INFO(this->get_logger(), "Publishing point cloud");
  }

  sensor_msgs::msg::PointCloud2 generate_point_cloud()
  {
    sensor_msgs::msg::PointCloud2 msg;
    msg.header.frame_id = "map";
    msg.header.stamp = this->now();
    msg.height = 1;
    msg.width = static_cast<uint32_t>((depth / resolution) * (width / resolution) * (height / resolution));

    // Define fields
    sensor_msgs::PointCloud2Modifier modifier(msg);
    modifier.setPointCloud2Fields(
        7,
        "x", 1, sensor_msgs::msg::PointField::FLOAT32,
        "y", 1, sensor_msgs::msg::PointField::FLOAT32,
        "z", 1, sensor_msgs::msg::PointField::FLOAT32,
        "r", 1, sensor_msgs::msg::PointField::FLOAT32,
        "g", 1, sensor_msgs::msg::PointField::FLOAT32,
        "b", 1, sensor_msgs::msg::PointField::FLOAT32,
        "a", 1, sensor_msgs::msg::PointField::FLOAT32
    );
    modifier.resize(msg.width);

    sensor_msgs::PointCloud2Iterator<float> iter_x(msg, "x");
    sensor_msgs::PointCloud2Iterator<float> iter_y(msg, "y");
    sensor_msgs::PointCloud2Iterator<float> iter_z(msg, "z");
    sensor_msgs::PointCloud2Iterator<float> iter_r(msg, "r");
    sensor_msgs::PointCloud2Iterator<float> iter_g(msg, "g");
    sensor_msgs::PointCloud2Iterator<float> iter_b(msg, "b");
    sensor_msgs::PointCloud2Iterator<float> iter_a(msg, "a");

    for (size_t i = 0; i < depth / resolution; ++i) {
      for (size_t j = 0; j < width / resolution; ++j) {
        for (size_t k = 0; k < height / resolution; ++k) {
          float x = i * resolution - depth * resolution / 2;
          float y = j * resolution - width * resolution / 2;
          float z = k * resolution - height * resolution / 2;

          *iter_x = x;
          *iter_y = y;
          *iter_z = z;

          float distance = std::sqrt(
              std::pow(x - sphere_center.x, 2) +
              std::pow(y - sphere_center.y, 2) +
              std::pow(z - sphere_center.z, 2)
          );

          if (distance <= sphere_radius) {
            // Inside 
            *iter_r = 0.0f; 
            *iter_g = 0.0f;
            *iter_b = 1.0f;
          } else {
  
            *iter_r = 1.0f; 
            *iter_g = 0.0f; 
            *iter_b = 0.0f;

          }
          
          ++iter_x; ++iter_y; ++iter_z;
          ++iter_r; ++iter_g; ++iter_b; ++iter_a;
        }
      }
    }

    return msg;
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr publisher_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PointCloudPublisher>());
  rclcpp::shutdown();
  return 0;
}
