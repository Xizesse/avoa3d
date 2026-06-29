// src/sample_evaluator.cpp
#include "s3vo/sample_evaluator.hpp"
#include <algorithm>
#include <cmath>


namespace s3vo {

SampleEvaluator::SampleEvaluator(rclcpp::Logger logger, double vehicle_radius, 
                                 double heading_weight, double danger_weight, double abs_weight, 
                                 double momentum_weight, double linear_matching_weight, double twist_matching_weight,
                                 double time_to_collision_threshold, double radius_threshold)
    : logger_(logger),
      vehicle_radius_(vehicle_radius),
        heading_weight_(heading_weight),
        danger_weight_(danger_weight),
        abs_weight_(abs_weight),
        momentum_weight_(momentum_weight),
        linear_matching_weight_(linear_matching_weight),
        twist_matching_weight_(twist_matching_weight),
        time_to_collision_threshold_(time_to_collision_threshold),
        radius_threshold_(radius_threshold)

{
    RCLCPP_INFO(logger_, "Initializing SampleEvaluator");
}

void SampleEvaluator::setDesiredVelocity(const geometry_msgs::msg::Twist& desired_velocity)
{
    latest_desired_velocity_ = desired_velocity;
}

void SampleEvaluator::evaluateSamples(std::vector<VelocitySample>& samples, const s3vo::msg::ElementCharacteristicsArray& obstacles, const geometry_msgs::msg::Twist& current_velocity)
{
    //* --- 1. PREPARE DESIRED VELOCITY CONTEXT ---
    double desired_vx = latest_desired_velocity_.linear.x;
    double desired_vy = latest_desired_velocity_.linear.y;
    double desired_vz = latest_desired_velocity_.linear.z;
    
    double desired_magnitude = std::sqrt(desired_vx * desired_vx + desired_vy * desired_vy + desired_vz * desired_vz);
    
    double desired_vx_norm = desired_vx;
    double desired_vy_norm = desired_vy;
    double desired_vz_norm = desired_vz;

    if (desired_magnitude > 0.001) { 
        desired_vx_norm /= desired_magnitude;
        desired_vy_norm /= desired_magnitude;
        desired_vz_norm /= desired_magnitude;
    }
    
    std::vector<VelocitySample> valid_samples;

    //! --- 2. SAMPLE EVALUATION LOOP ---
    for (auto& sample : samples) {
        sample.cost = 0.0;
        sample.danger = 0.0;
        sample.heading = 0.0;
        sample.magnitude = 0.0;
        sample.momentum = 0.0;
        sample.linear_matching = 0.0;
        sample.twist_matching = 0.0;

        bool collision_free = true;

        double sm_x = sample.velocity_space.x;
        double sm_y = sample.velocity_space.y;
        double sm_z = sample.velocity_space.z;
        double sample_magnitude = std::sqrt(sm_x * sm_x + sm_y * sm_y + sm_z * sm_z);

        //* --- 2.1 OBSTACLE PRUNING ---
        for (const auto& obstacle : obstacles.elements) {
            double rel_vx = sm_x - obstacle.velocity.x;
            double rel_vy = sm_y - obstacle.velocity.y;
            double rel_vz = sm_z - obstacle.velocity.z;
            
            double obs_x = obstacle.pose.position.x;
            double obs_y = obstacle.pose.position.y;
            double obs_z = obstacle.pose.position.z;
            double obs_dist = std::sqrt(obs_x * obs_x + obs_y * obs_y + obs_z * obs_z);

            if (obs_dist < 0.001) { collision_free = false; break; }

            double obs_radius = std::max({obstacle.size.x, obstacle.size.y, obstacle.size.z}) / 2.0 
                                + vehicle_radius_ + obstacle.protective_zone;

            // Check if we are already inside the obstacle boundary
            if (obs_dist <= obs_radius) {
                double p_dot_v = rel_vx * obs_x + rel_vy * obs_y + rel_vz * obs_z;
                if (p_dot_v > 0.0) { // Moving deeper/towards obstacle center
                    collision_free = false;
                    break;
                }
                // Moving away is allowed to escape
                continue; 
            }

            // Quadratic coefficients for sphere-ray intersection: a*t^2 + b*t + c = 0
            // a = ||v||^2
            double a = rel_vx * rel_vx + rel_vy * rel_vy + rel_vz * rel_vz;
            // p_dot_v = p . v
            double p_dot_v = rel_vx * obs_x + rel_vy * obs_y + rel_vz * obs_z;

            if (a >= 0.001 && p_dot_v > 0.0) {
                // b = -2 * (p . v)
                double b = -2.0 * p_dot_v;
                // c = ||p||^2 - R^2
                double c = obs_dist * obs_dist - obs_radius * obs_radius;
                
                double discriminant = b * b - 4.0 * a * c;
                if (discriminant >= 0.0) {
                    double t_entry = (-b - std::sqrt(discriminant)) / (2.0 * a);
                    if (t_entry > 0.0 && t_entry < time_to_collision_threshold_) {
                        collision_free = false;
                        break;
                    }
                }
            }

            // Retain original cone and ttc calculations strictly for the danger weight heuristic
            double rel_mag_sq = rel_vx * rel_vx + rel_vy * rel_vy + rel_vz * rel_vz;
            double projection = (rel_vx * obs_x + rel_vy * obs_y + rel_vz * obs_z) / obs_dist;
            double cone_angle = std::asin(obs_radius / obs_dist);
            double expected_radius = projection * std::tan(cone_angle);
            double radius_squared = rel_mag_sq - (projection * projection);
            double actual_radius = (radius_squared >= 0.0) ? std::sqrt(radius_squared) : 0.0;
            double ttc = (obs_dist - obs_radius) / std::max(0.001, projection); 

            if (danger_weight_ > 0.0 && projection > 0.0) {
                double dist_to_boundary = actual_radius - expected_radius;
                if (dist_to_boundary < radius_threshold_ && ttc < time_to_collision_threshold_) {
                    double danger_val = ((radius_threshold_ - dist_to_boundary) / radius_threshold_) * projection;
                    sample.danger = std::min(1.0, sample.danger + danger_val);
                }
            }
        }

        if (!collision_free) continue;

        //* --- 2.2 NAVIGATION HEURISTICS ---

        if (heading_weight_ > 0.0) {
            double h_cost = 1.0; 
            if (sample_magnitude >= 0.01) {
                double dot = (sm_x/sample_magnitude * desired_vx_norm) + 
                             (sm_y/sample_magnitude * desired_vy_norm) + 
                             (sm_z/sample_magnitude * desired_vz_norm);
                h_cost = ((std::max(-1.0, std::min(1.0, dot)) - 1.0) / -2.0);
            }
            sample.heading = h_cost;
        }

        if (abs_weight_ > 0.0) {
            double max_speed = 1.0; 
            sample.magnitude = std::abs(sample_magnitude - desired_magnitude) / max_speed;
        }

        if (linear_matching_weight_ > 0.0) {
            double l_dx = sm_x - desired_vx;
            double l_dy = sm_y - desired_vy;
            double l_dz = sm_z - desired_vz;
            sample.linear_matching = std::sqrt(l_dx*l_dx + l_dy*l_dy + l_dz*l_dz);
        }

        if (twist_matching_weight_ > 0.0) {
            double dv_x = std::abs(sample.twist.lx - latest_desired_velocity_.linear.x);
            double dw_z = std::abs(sample.twist.az - latest_desired_velocity_.angular.z);
            sample.twist_matching = (dv_x / 1.0) + (dw_z / 1.0); 
        }

        if (momentum_weight_ > 0.0) {
            if (sample_magnitude < 0.01) { sample.momentum = 0.0; }
            else {
                double curr_mag = std::sqrt(current_velocity.linear.x * current_velocity.linear.x +
                                            current_velocity.linear.y * current_velocity.linear.y +
                                            current_velocity.linear.z * current_velocity.linear.z);
                if (curr_mag >= 0.01) {
                    double dot_m = (sm_x/sample_magnitude * current_velocity.linear.x/curr_mag) +
                                   (sm_y/sample_magnitude * current_velocity.linear.y/curr_mag) +
                                   (sm_z/sample_magnitude * current_velocity.linear.z/curr_mag);
                    sample.momentum = ((1.0 - std::max(-1.0, std::min(1.0, dot_m))) / 2.0);
                }
            }
        }

        //* --- 2.3 FINAL COST AGGREGATION ---
        double goal_cost = heading_weight_         * sample.heading + 
                           abs_weight_             * sample.magnitude + 
                           momentum_weight_        * sample.momentum +
                           linear_matching_weight_ * sample.linear_matching +
                           twist_matching_weight_  * sample.twist_matching;

        sample.cost = (1.0 - danger_weight_) * goal_cost + danger_weight_ * sample.danger;
        valid_samples.push_back(sample);
    }

    //* --- 3. SELECTION ---
    if (!valid_samples.empty()) {
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

} // namespace s3vo