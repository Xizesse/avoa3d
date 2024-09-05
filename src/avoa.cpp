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
        width_cells_(static_cast<int>(width_m_ / resolution_m)),
        depth_cells_(static_cast<int>(depth_m_ / resolution_m)),
        {
            timer_ = this->create_wall_timer(1000ms, std::bind(&avoa::timer_callback, this));
        }
    private:

        void timer_callback()
        {
            RCLCPP_INFO(this->get_logger(), "Loop");
            //! Subscrever a posição e a velocidade do agente
            //! Subscrever a posição e a velocidade do obstaculo 


            //! Criar o espaço com as coordenadas do agente + offset do espaço
            
            //!PSO
            //! A cada elemento
                //! Aceleração máxima
                //! Restrições de 
                //! Verificar geometricamente se é protective zone
                //! Marcar se for protective zone

            //! 
            //! 


        }
    
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<avoa>());
    rclcpp::shutdown();
    return 0;
}