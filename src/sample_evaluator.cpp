// src/sample_evaluator.cpp
#include "avoa3d/sample_evaluator.hpp"
#include <algorithm>
#include <cmath>


namespace avoa3d {

SampleEvaluator::SampleEvaluator(rclcpp::Logger logger, double vehicle_radius, 
                                 double heading_weight, double danger_weight, double abs_weight, double momentum_weight, double time_to_collision_threshold, double radius_threshold)
    : logger_(logger),
      vehicle_radius_(vehicle_radius),
        heading_weight_(heading_weight),
        danger_weight_(danger_weight),
        abs_weight_(abs_weight),
        momentum_weight_(momentum_weight),
        time_to_collision_threshold_(time_to_collision_threshold),
        radius_threshold_(radius_threshold)

{
    RCLCPP_INFO(logger_, "Initializing SampleEvaluator with vehicle radius: %.2f", vehicle_radius_);
    RCLCPP_INFO(logger_, "Heading weight: %.2f", heading_weight_);
    RCLCPP_INFO(logger_, "Danger weight: %.2f", danger_weight_);
    RCLCPP_INFO(logger_, "Abs weight: %.2f", abs_weight_);
    RCLCPP_INFO(logger_, "Momentum weight: %.2f", momentum_weight_);
}

void SampleEvaluator::setDesiredVelocity(const geometry_msgs::msg::Twist& desired_velocity)
{
    latest_desired_velocity_ = desired_velocity;
}

void SampleEvaluator::evaluateSamples(std::vector<VelocitySample>& samples, const avoa3d::msg::ElementCharacteristicsArray& obstacles, const geometry_msgs::msg::Twist& current_velocity)
{
    
    double desired_vx, desired_vy, desired_vz;
    
    double dt = 1.0; //TODO: Make this a parameteR

    // SAMPLES ARE ALLWAYS XYZ
    desired_vx = latest_desired_velocity_.linear.x;
    desired_vy = latest_desired_velocity_.linear.y;
    desired_vz = latest_desired_velocity_.linear.z;
    
    double desired_magnitude = std::sqrt(desired_vx * desired_vx + desired_vy * desired_vy + desired_vz * desired_vz);
    
    if (desired_magnitude > 0.001) { //TODO ELSE ?
        desired_vx /= desired_magnitude;
        desired_vy /= desired_magnitude;
        desired_vz /= desired_magnitude;
    }
    
    std::vector<VelocitySample> valid_samples;
    VelocitySample translated_sample;
    
    // Safety cost parameters
    const double safety_threshold = 0.3;   // TODO: Make this a parameter
    const double max_safety_cost = 0.3;    // TODO: make this a parameter
    
    //! For each sample 
    for (auto& sample : samples) {
        bool collision_free = true;
        sample.cost = 0.0;
        sample.danger = 0.0;

        //NON ZERO LINEAR SAMPLES 
        if (!(sample.vx == 0.0 && sample.vy == 0.0 && sample.vz == 0.0)) { 

            //Obstacle loop
            for (const auto& obstacle : obstacles.elements) {
            
                //!Relative velocity
                translated_sample.vx = sample.vx - obstacle.velocity.x ;
                translated_sample.vy = sample.vy - obstacle.velocity.y ;
                translated_sample.vz = sample.vz - obstacle.velocity.z ;
                
                double obstacle_x = obstacle.pose.position.x;
                double obstacle_y = obstacle.pose.position.y;
                double obstacle_z = obstacle.pose.position.z;

                // get obstacle radius
                double obstacle_radius = std::max({obstacle.size.x / 2.0, obstacle.size.y / 2.0, obstacle.size.z / 2.0}) 
                                        + vehicle_radius_ + obstacle.protective_zone; 

                //std::cout << "Obstacle radius: " << obstacle_radius << std::endl;

                //get obstacle distance
                double obstacle_distance = std::sqrt(
                    obstacle_x * obstacle_x +
                    obstacle_y * obstacle_y +
                    obstacle_z * obstacle_z
                );
                
                if (obstacle_distance < 0.001) {
                    collision_free = false;
                    break;
                }

                //TGet Sample Distance
                double sample_distance = std::sqrt(
                    translated_sample.vx * translated_sample.vx +
                    translated_sample.vy * translated_sample.vy +
                    translated_sample.vz * translated_sample.vz
                );

                double projection = (translated_sample.vx * obstacle_x + translated_sample.vy * obstacle_y + translated_sample.vz * obstacle_z) / obstacle_distance;
                
                double cone_angle = 0.0;
                
                if (obstacle_distance <= obstacle_radius)
                {
                    cone_angle = M_PI / 2.0;   
                    
                    // If projection is positive, we are moving deeper into the obstacle
                    if (projection > 0.0)
                    {
                        collision_free = false;
                        break;
                    }
                }
                else
                {
                    cone_angle = std::asin(obstacle_radius / obstacle_distance);
                }
                
                //Get the projection of the translated sample on the obstacle axis

                // Calculate the expected radius in the cone :
                double expected_radius = projection * std::tan(cone_angle);

                //std::cout << "Expected radius: " << expected_radius << std::endl;

                // Calculate the actual distance to the axis (use the angle between the translated sample and the obstacle, and the distance to the sample
                double radius_squared = sample_distance * sample_distance - projection * projection;
                double actual_radius = (radius_squared >= 0.0) ? std::sqrt(radius_squared) : 0.0;
                //std::cout << "Actual radius: " << actual_radius << std::endl;
                
                // Use projection (velocity towards obstacle) instead of full sample distance
                float time_to_collision = (obstacle_distance - obstacle_radius) / std::max(0.001, projection);
                
                if (actual_radius < expected_radius) { //if Collision Cone
                    // Collision detected -> Break to Remove
                    
                    /*//! Time Horizon Constraint
                    If the time for collision is greater than X seconds, we dont remove the sample
                    sample distance = magnitude velocity
                    obstacle_distance - obstacle_radius = distance for collision
                    defined time
                    check if the time for collision is greater than the threshold 
                    */
                    
                    if (time_to_collision < time_to_collision_threshold_)
                    {
                        collision_free = false;
                        break;
                    }
                    else {
                        
                    }
                } 
                if (projection > 0.0 && actual_radius - expected_radius < radius_threshold_ ) {
                    if((time_to_collision < time_to_collision_threshold_)) // above this threshold
                    {
                        sample.danger += (( radius_threshold_ - (actual_radius - expected_radius)) / radius_threshold_)  *projection;
                        sample.danger = std::min(1.0, sample.danger);
                    }
                    else
                    {   
                        float truncated_min_distance = (obstacle_distance - obstacle_radius) / time_to_collision_threshold_;
                        float truncated_center_x = (obstacle_x/obstacle_distance)* truncated_min_distance;
                        float truncated_center_y = (obstacle_y/obstacle_distance)* truncated_min_distance;
                        float truncated_center_z = (obstacle_z/obstacle_distance)* truncated_min_distance;
                    
                        float distance_to_truncated_center = std::sqrt(
                            (truncated_center_x - translated_sample.vx) * (truncated_center_x - translated_sample.vx) +
                            (truncated_center_y - translated_sample.vy) * (truncated_center_y - translated_sample.vy) +
                            (truncated_center_z - translated_sample.vz) * (truncated_center_z - translated_sample.vz)
                        );
                        double truncated_expected_radius =  radius_threshold_ + (truncated_min_distance/obstacle_distance)*obstacle_radius;

                        if (distance_to_truncated_center < truncated_expected_radius )
                        {
                            //sample.danger += (truncated_expected_radius - distance_to_truncated_center) / truncated_expected_radius;
                        }
            
                    }
                    
                }
            
    
        }   
        else {
            // TODO NOT Skip zero velocity samples
            
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

            if (sample_magnitude >= 0.01) {  // Avoid division by zero for normalization
                // Normalize the sample velocity
                double sample_nx = sample.vx / sample_magnitude;
                double sample_ny = sample.vy / sample_magnitude;
                double sample_nz = sample.vz / sample_magnitude;
                
                // desired_vx/y/z are already normalized at the start of the function, 
                // so we don't divide by desired_magnitude here!
                double dot_product = (sample_nx * desired_vx + sample_ny * desired_vy + sample_nz * desired_vz);
                
                dot_product = std::max(-1.0, std::min(1.0, dot_product));
                
                direction_error = (dot_product - 1.0) / -2.0;  // Normalize to [0, 1]
            } else {
                continue;  // No direction error if sample velocity is zero
            }
            
            // TODO 2. Magnitude error (difference between magnitudes)

            double magnitude_error = std::abs(sample_magnitude - desired_magnitude);
            // 3. Combine errors 
            // Normalize magnitude error by dividing by desired magnitude (if non-zero)
            double max_speed = 1.0;
            double normalized_magnitude_error = magnitude_error / max_speed;
            
            // To prevent oscillation/chattering, we penalize drastic changes in velocity direction!
            // We calculate how much this sample diverges from the CURRENT velocity context.
            double momentum_error = 0.0;
            double current_magnitude = std::sqrt(current_velocity.linear.x * current_velocity.linear.x +
                                                 current_velocity.linear.y * current_velocity.linear.y +
                                                 current_velocity.linear.z * current_velocity.linear.z);
            if (current_magnitude >= 0.01 && sample_magnitude >= 0.01) {
                double curr_nx = current_velocity.linear.x / current_magnitude;
                double curr_ny = current_velocity.linear.y / current_magnitude;
                double curr_nz = current_velocity.linear.z / current_magnitude;
                
                double sample_nx = sample.vx / sample_magnitude;
                double sample_ny = sample.vy / sample_magnitude;
                double sample_nz = sample.vz / sample_magnitude;
                
                double dot_momentum = sample_nx * curr_nx + sample_ny * curr_ny + sample_nz * curr_nz;
                dot_momentum = std::max(-1.0, std::min(1.0, dot_momentum));
                momentum_error = (1.0 - dot_momentum) / 2.0; // [0, 1], 0 means keep same direction
            }
            
            // Temporary small weight for momentum to fix oscillation. 
            // This favors keeping the same avoidance side instead of switching back and forth.
            // Using parameterized momentum_weight_ 

            // ! Combine goal-directed cost with safety cost and momentum
            double goal_cost = heading_weight_ * direction_error + 
                               abs_weight_ * normalized_magnitude_error + 
                               momentum_weight_ * momentum_error;
            
            sample.cost = (1-danger_weight_) * goal_cost + danger_weight_ * sample.danger;
             
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