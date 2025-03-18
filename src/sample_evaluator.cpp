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

void SampleEvaluator::evaluateSamples(std::vector<VelocitySample>& samples, const custom_msgs::msg::ElementCharacteristicsArray& obstacles, double delta_t_)
{
    double desired_vx = latest_desired_velocity_.linear.x;
    double desired_vy = latest_desired_velocity_.linear.y;
    double desired_vz = latest_desired_velocity_.linear.z;
    
    double desired_magnitude = std::sqrt(
        desired_vx * desired_vx + 
        desired_vy * desired_vy + 
        desired_vz * desired_vz
    );
    
    // Normalize the desired velocity vector (if non-zero)
    if (desired_magnitude > 0.001) {
        desired_vx /= desired_magnitude;
        desired_vy /= desired_magnitude;
        desired_vz /= desired_magnitude;

    }
    
    // Vector to keep track of valid (collision-free) samples
    std::vector<VelocitySample> valid_samples;
    // SIngleTranslated sample
    VelocitySample translated_sample;
    
    // Process each sample
    for (auto& sample : samples) {
        // Check if this velocity would lead to a collision with any obstacle
        bool collision_free = true;
        
        
        for (const auto& obstacle : obstacles.elements) {

            
            translated_sample.vx = sample.vx - obstacle.velocity.x * delta_t_;
            translated_sample.vy = sample.vy - obstacle.velocity.y * delta_t_;
            translated_sample.vz = sample.vz - obstacle.velocity.z * delta_t_;

            // Get obstacle position (already in agent frame)
            double obstacle_x = obstacle.pose.position.x;
            double obstacle_y = obstacle.pose.position.y;
            double obstacle_z = obstacle.pose.position.z;

            // Effective radius (including protective zone)
            double obstacle_radius = std::max({
                obstacle.size.x / 2.0,
                obstacle.size.y / 2.0,
                obstacle.size.z / 2.0
            }) + vehicle_radius_ + obstacle.protective_zone;
            
            // Distance from agent to obstacle (agent is at origin in agent frame)
            double distance = std::sqrt(
                obstacle_x * obstacle_x +
                obstacle_y * obstacle_y +
                obstacle_z * obstacle_z
            );
            
            // Vector pointing to obstacle (normalized)
            double to_obstacle_x = obstacle_x;
            double to_obstacle_y = obstacle_y;
            double to_obstacle_z = obstacle_z;
            
            // Normalize the vector to obstacle
            if (distance > 0.001) {
                to_obstacle_x /= distance;
                to_obstacle_y /= distance;
                to_obstacle_z /= distance;
            } else {
                // We're practically inside the obstacle, all velocities lead to collision
                collision_free = false;
                break;
            }
            
            // Projection of velocity onto the direction to obstacle
            double velocity_projection = translated_sample.vx * to_obstacle_x + 
                                        translated_sample.vy * to_obstacle_y + 
                                        translated_sample.vz * to_obstacle_z;
            
            // If moving away from obstacle, no collision
            if (velocity_projection <= 0) {
                continue;
            }
            
            // Calculate the magnitude of the sample velocity
            double sample_magnitude = std::sqrt(
                translated_sample.vx * translated_sample.vx + 
                translated_sample.vy * translated_sample.vy + 
                translated_sample.vz * translated_sample.vz
            );
            
            // Avoid division by zero
            if (sample_magnitude < 0.001) {
                continue;  // Zero velocity won't cause a collision
            }
            
            // Calculate the perpendicular component (closest approach vector)
            double perp_x = to_obstacle_x - (velocity_projection / sample_magnitude) * (translated_sample.vx / sample_magnitude);
            double perp_y = to_obstacle_y - (velocity_projection / sample_magnitude) * (translated_sample.vy / sample_magnitude);
            double perp_z = to_obstacle_z - (velocity_projection / sample_magnitude) * (translated_sample.vz / sample_magnitude);
            
            double closest_approach_distance = std::sqrt(
                perp_x * perp_x +
                perp_y * perp_y +
                perp_z * perp_z
            ) * distance;
            
            // Check if this velocity is inside the collision cone
            if (closest_approach_distance < obstacle_radius) {
                collision_free = false;
                break;
            }
        }
        
        // If this velocity doesn't lead to collision, keep it and evaluate cost
        if (collision_free) {
            // Calculate magnitude of the sample velocity
            double sample_magnitude = std::sqrt(
                sample.vx * sample.vx + 
                sample.vy * sample.vy + 
                sample.vz * sample.vz
            );
            
            // 1. Direction error (heading alignment with desired velocity)
            double direction_error = 0.0;
            if (true) {  // TODO Avoid division by zero for normalization
                // Normalize the sample velocity
                double sample_nx = sample.vx / sample_magnitude;
                double sample_ny = sample.vy / sample_magnitude;
                double sample_nz = sample.vz / sample_magnitude;
                
                // Dot product gives cosine of angle between vectors
                double dot_product = sample_nx * desired_vx + sample_ny * desired_vy + sample_nz * desired_vz;
                
                // Constrain dot product to [-1, 1] range to avoid numerical issues
                dot_product = std::max(-1.0, std::min(1.0, dot_product));
                
                // Convert to angle error (0 means perfect alignment, π means opposite direction)
                direction_error = std::acos(dot_product);
            } else {
                // If sample velocity is zero but desired is not, this is a mismatch
                direction_error = M_PI;  // Maximum direction error
            }
            
            // 2. Magnitude error (difference between magnitudes)
            double magnitude_error = std::abs(sample_magnitude - desired_magnitude);
            
            // 3. Combine errors - weight direction error more heavily (70%) than magnitude error (30%)
            // Normalize magnitude error by dividing by desired magnitude (if non-zero)
            double normalized_magnitude_error = (desired_magnitude > 0.001) ? 
                magnitude_error / desired_magnitude : magnitude_error;
                
            sample.cost = 0.7 * direction_error + 0.3 * normalized_magnitude_error;
            
            valid_samples.push_back(sample);
        }
    }
    
    // Check if we have any valid samples
    if (valid_samples.empty()) {
        RCLCPP_WARN(logger_, "All velocity samples would lead to collision: Das ist nicht gut");
        
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