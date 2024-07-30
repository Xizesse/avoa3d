#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class avoa : public rclcpp::Node
{
    public:
        avoa():Node("avoa")
        height_m_(15.0),
        width_m_(30.0),
        depth_m_(30.0),
        resolution_m(0.2f)
        height_cells_(static_cast<int>(height_m / resolution_m)),
        width_cells_(static_cast<int>(width_m_ / resolution_)),
        depth_cells_(static_cast<int>(depth_m_ / resolution_)),
        space_(height_cells_, std::vector<std::vector<int>>(width_cells_, std::vector<int>(depth_cells_, 0)))

        {
            timer_ = this->create_wall_timer(1000ms, std::bind(&avoa::timer_callback, this));
        }
    private:
        void timer_callback()
        {
            RCLCPP_INFO(this->get_logger(), "Hello, world!");
        }
    float height_m_;
    float width_m_;
    float depth_m_;
    float resolution_;
    int height_cells_;
    int width_cells_;
    int depth_cells_;
    std::vector<std::vector<std::vector<int>>> space_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<avoa>());
    rclcpp::shutdown();
    return 0;
}