// src/sample_evaluator.cpp
#include "avoa3d/sample_evaluator.hpp"
#include <algorithm>
#include <cmath>

namespace avoa3d {

SampleEvaluator::SampleEvaluator(rclcpp::Logger logger, double vehicle_radius)
    : logger_(logger),
      vehicle_radius_(vehicle_radius)
{
}

void SampleEvaluator::setDesiredVelocity(const geometry_msgs::msg::Twist& desired_velocity)
{
    latest_desired_velocity_ = desired_velocity;
}

void SampleEvaluator::evaluateSamples(std::vector<VelocitySample>& samples, const custom_msgs::msg::ElementCharacteristicsArray& obstacles )
{
    double desired_vx = latest_desired_velocity_.linear.x;
    double desired_vy = latest_desired_velocity_.linear.y;
    double desired_vz = latest_desired_velocity_.linear.z;
    
    double desired_magnitude = std::sqrt(desired_vx * desired_vx + desired_vy * desired_vy + desired_vz * desired_vz);
    
    if (desired_magnitude > 0.001) {
        desired_vx /= desired_magnitude;
        desired_vy /= desired_magnitude;
        desired_vz /= desired_magnitude;
    }
    
    std::vector<VelocitySample> valid_samples;
    VelocitySample translated_sample;
    
    // Safety cost parameters
    const double safety_threshold = 3.0;   // Maximum distance to apply safety cost
    const double max_safety_cost = 1.0;    // Maximum safety cost when on cone boundary
    
    for (auto& sample : samples) {
        bool collision_free = true;
        sample.cost = 0.0;
        sample.danger = 0.0;

        for (const auto& obstacle : obstacles.elements) {

            //std::cout << "\nStarting New Sample" << std::endl;
            translated_sample.vx = sample.vx - obstacle.velocity.x ;
            translated_sample.vy = sample.vy - obstacle.velocity.y ;
            translated_sample.vz = sample.vz - obstacle.velocity.z ;

            //std::cout << "Translated Sample Coordinates" << std::endl;
            //std::cout << "Translated Sample vx: " << translated_sample.vx << std::endl;
            //std::cout << "Translated Sample vy: " << translated_sample.vy << std::endl;
            //std::cout << "Translated Sample vz: " << translated_sample.vz << std::endl;
            
            double obstacle_x = obstacle.pose.position.x;
            double obstacle_y = obstacle.pose.position.y;
            double obstacle_z = obstacle.pose.position.z;
            //std::cout << "Obstacle x: " << obstacle_x << std::endl;
            //std::cout << "Obstacle y: " << obstacle_y << std::endl;
            //std::cout << "Obstacle z: " << obstacle_z << std::endl;
    


            // get obstacle radius
            double obstacle_radius = std::max({obstacle.size.x / 2.0, obstacle.size.y / 2.0, obstacle.size.z / 2.0}) 
                                    + vehicle_radius_ * 1.8 + obstacle.protective_zone; 

            //std::cout << "Obstacle radius: " << obstacle_radius << std::endl;

            //get obstacle distance
            double obstacle_distance = std::sqrt(
                obstacle_x * obstacle_x +
                obstacle_y * obstacle_y +
                obstacle_z * obstacle_z
            );

            //std::cout << "Obstacle distance: " << obstacle_distance << std::endl;

            //TGet Sample Distance
            double sample_distance = std::sqrt(
                translated_sample.vx * translated_sample.vx +
                translated_sample.vy * translated_sample.vy +
                translated_sample.vz * translated_sample.vz
            );

            //std::cout << "Sample Coordinates" << std::endl;

            //std::cout << "Sample distance: " << sample_distance << std::endl;
            // Get Cone Angle
            double cone_angle = std::asin(obstacle_radius / obstacle_distance);

            //std::cout << "Cone angle: " << cone_angle << std::endl;
    
            
            //Get the projection of the translated sample on the obstacle axis
            double projection = (translated_sample.vx * obstacle_x + translated_sample.vy * obstacle_y + translated_sample.vz * obstacle_z) / obstacle_distance;

            //std::cout << "Projection size: " << projection << std::endl;

            if (projection < 0) {
                // The obstacle is behind the sample, no need to check
                continue;
            }

            // Calculate the expected radius in the cone :
            double expected_radius = projection * std::tan(cone_angle);

            //std::cout << "Expected radius: " << expected_radius << std::endl;

            // Calculate the actual distance to the axis (use the angle between the translated sample and the obstacle, and the distance to the sample
            double actual_radius = std::sqrt(sample_distance * sample_distance - projection * projection);

            //std::cout << "Actual radius: " << actual_radius << std::endl;
            
            if (actual_radius < expected_radius) {
                // Collision detected
                collision_free = false;
                break;
            }
            double radius_treshold = 2.0;

            if (actual_radius - expected_radius > radius_treshold) {
                // The sample is far from the obstacle, dw
                continue;
            }
            // Calculate the safety cost
            sample.danger += (2.0 - (actual_radius - expected_radius))*0.5;


    
        }
        
        // If this velocity doesn't lead to collision, keep it and evaluate cost
        if (collision_free) {
            // Calculate magnitude of the sample velocity
            double sample_magnitude = std::sqrt(
                sample.vx * sample.vx + 
                sample.vy * sample.vy + 
                sample.vz * sample.vz
            );
            
            // TODO 1. Direction error (heading alignment with desired velocity)
            double direction_error = 0.0;

            if (sample_magnitude > 0.001) {  // Avoid division by zero for normalization
                // Normalize the sample velocity
                double sample_nx = sample.vx / sample_magnitude;
                double sample_ny = sample.vy / sample_magnitude;
                double sample_nz = sample.vz / sample_magnitude;
                
                double dot_product = (sample_nx * desired_vx + sample_ny * desired_vy + sample_nz * desired_vz );
                
                dot_product = std::max(-1.0, std::min(1.0, dot_product));//clamp to [-1,1]
                
                direction_error = std::acos(dot_product)/M_PI;  // Normalize to [0,1]


            } else {
                direction_error = 0;  // No direction error if sample velocity is zero
            }
            
            // TODO 2. Magnitude error (difference between magnitudes)
            double magnitude_error = std::abs(sample_magnitude - desired_magnitude);
            
            // 3. Combine errors 
            // Normalize magnitude error by dividing by desired magnitude (if non-zero)
            double normalized_magnitude_error = (desired_magnitude > 0.001) ? 
                magnitude_error / desired_magnitude : magnitude_error;
            
            std::atan2(sample.vy,sample.vx);

            // Combine goal-directed cost with safety cost
            double goal_cost = 0.5 * direction_error + 0.5* normalized_magnitude_error;
            
            sample.cost = 1.0 * goal_cost + 0.0 * sample.danger ; 
            
            if (sample.vy <= 0.0 )
            {
                sample.cost -= 0.1*sample.vy/sample_magnitude;
            }
             
            valid_samples.push_back(sample); 
        }
    }
    
    
    // Check if we have any valid samples
    if (valid_samples.empty()) {
        // No valid samples found, keep the original samples
        RCLCPP_WARN(logger_, "No collision-free velocity samples found");
    } else {
        // Replace the original samples with only the valid ones
        samples = valid_samples;
    }
}

VelocitySample SampleEvaluator::findBestSample(const std::vector<VelocitySample>& samples)
{
    auto min_element = std::min_element(
        samples.begin(), samples.end(),
        [](const VelocitySample& a, const VelocitySample& b) { return a.cost < b.cost; }
    );
    
    return *min_element;
}

} // namespace avoa3d