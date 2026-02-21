#include "avoa3d/sample_generator.hpp"
#include <cmath>

namespace avoa3d
{

//!HOLONOMIC SAMPLE GENERATOR

HolonomicSampleGenerator::HolonomicSampleGenerator(const rclcpp::Logger& logger)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing holonomic sample generator with default parameters");
    // Default parameters already initialized in MotionParameters struct
}

HolonomicSampleGenerator::HolonomicSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing holonomic sample generator");
    // Parameters will be set by the AVOA node
}

void HolonomicSampleGenerator::setMotionParameters(const MotionParameters& params)
{
    params_ = params;
    
    RCLCPP_INFO(logger_, "Set holonomic parameters:");
    RCLCPP_INFO(logger_, "  a_x_max: %.2f", params_.a_x_max);
    RCLCPP_INFO(logger_, "  a_y_max: %.2f", params_.a_y_max);
    RCLCPP_INFO(logger_, "  a_z_max: %.2f", params_.a_z_max);
    RCLCPP_INFO(logger_, "  v_x_max: %.2f", params_.v_x_max);
    RCLCPP_INFO(logger_, "  v_y_max: %.2f", params_.v_y_max);
    RCLCPP_INFO(logger_, "  v_z_max: %.2f", params_.v_z_max);
    RCLCPP_INFO(logger_, "  delta_t: %.2f", params_.delta_t);
    RCLCPP_INFO(logger_, "  num_samples: %d", params_.num_samples);
}

std::vector<VelocitySample> HolonomicSampleGenerator::generateSamples(
    const geometry_msgs::msg::Twist& current_velocity,
    const geometry_msgs::msg::Twist& desired_velocity)
    {
        std::vector<VelocitySample> samples;
        std::random_device rd;
        // random number generation
        std::mt19937 gen(rd());
        
        // Calculate velocity limits based on current velocity and acceleration constraints
        // Lower bounds: current velocity - max acceleration * delta_t (but not below -v_max)
        // Upper bounds: current velocity + max acceleration * delta_t (but not above v_max)

        double min_vx = std::max(current_velocity.linear.x - params_.a_x_max * params_.delta_t, -params_.v_x_max);
        double max_vx = std::min(current_velocity.linear.x + params_.a_x_max * params_.delta_t, params_.v_x_max);
        
        double min_vy = std::max(current_velocity.linear.y - params_.a_y_max * params_.delta_t, -params_.v_y_max);
        double max_vy = std::min(current_velocity.linear.y + params_.a_y_max * params_.delta_t, params_.v_y_max);
        
        double min_vz = std::max(current_velocity.linear.z - params_.a_z_max * params_.delta_t, -params_.v_z_max);
        double max_vz = std::min(current_velocity.linear.z + params_.a_z_max * params_.delta_t, params_.v_z_max);
    
        // Create distributions based on the calculated bounds
        std::uniform_real_distribution<> dist_vx(min_vx, max_vx);
        std::uniform_real_distribution<> dist_vy(min_vy, max_vy);
        std::uniform_real_distribution<> dist_vz(min_vz, max_vz);
        
        // Generate random samples
        for (int i = 0; i < params_.num_samples; ++i) {
            double vx = dist_vx(gen);
            double vy = dist_vy(gen);
            double vz = dist_vz(gen);
            
            samples.push_back(VelocitySample(vx, vy, vz));
        }
        
        // Always include the current velocity as a sample
        if (current_velocity.linear.x >= min_vx && current_velocity.linear.x <= max_vx &&
            current_velocity.linear.y >= min_vy && current_velocity.linear.y <= max_vy &&
            current_velocity.linear.z >= min_vz && current_velocity.linear.z <= max_vz) {
            samples.push_back(VelocitySample(
                current_velocity.linear.x,
                current_velocity.linear.y,
                current_velocity.linear.z
            ));
        }
        
        
        // Always include the desired velocity as a sample 
        if (desired_velocity.linear.x >= min_vx && desired_velocity.linear.x <= max_vx &&
            desired_velocity.linear.y >= min_vy && desired_velocity.linear.y <= max_vy &&
            desired_velocity.linear.z >= min_vz && desired_velocity.linear.z <= max_vz) {
            samples.push_back(VelocitySample(
                desired_velocity.linear.x,
                desired_velocity.linear.y,
                desired_velocity.linear.z
            ));
        }
        
        return samples;
    }

geometry_msgs::msg::Twist HolonomicSampleGenerator::translateToTwist(const VelocitySample& sample)
{
    //return as is
    geometry_msgs::msg::Twist twist;
    twist.linear.x = sample.vx;
    twist.linear.y = sample.vy;
    twist.linear.z = sample.vz;
    twist.angular.x = 0.0;
    twist.angular.y = 0.0;
    twist.angular.z = 0.0;
    return twist;
}

void HolonomicSampleGenerator::setParams(
    double v_x_max, double v_y_max, double v_z_max,
    double a_x_max, double a_y_max, double a_z_max,
    double w_roll_max, double w_pitch_max, double w_yaw_max,
    double a_roll_max, double a_pitch_max, double a_yaw_max,
    double delta_t, int num_samples) 
{
    // Store parameters in the params_ structure
    params_.v_x_max = v_x_max;
    params_.v_y_max = v_y_max;
    params_.v_z_max = v_z_max;
    params_.a_x_max = a_x_max;
    params_.a_y_max = a_y_max;
    params_.a_z_max = a_z_max;
    params_.w_roll_max = w_roll_max;
    params_.w_pitch_max = w_pitch_max;
    params_.w_yaw_max = w_yaw_max;
    params_.a_roll_max = a_roll_max;
    params_.a_pitch_max = a_pitch_max;
    params_.a_yaw_max = a_yaw_max;
    params_.delta_t = delta_t;
    params_.num_samples = num_samples;
    
    // Log parameters
    RCLCPP_INFO(logger_, "Set holonomic parameters:");
    RCLCPP_INFO(logger_, "  Linear velocity limits [X,Y,Z]: [%.2f, %.2f, %.2f]", 
               params_.v_x_max, params_.v_y_max, params_.v_z_max);
    RCLCPP_INFO(logger_, "  Linear accel limits [X,Y,Z]: [%.2f, %.2f, %.2f]", 
               params_.a_x_max, params_.a_y_max, params_.a_z_max);
    RCLCPP_INFO(logger_, "  Angular velocity limits [R,P,Y]: [%.2f, %.2f, %.2f]", 
               params_.w_roll_max, params_.w_pitch_max, params_.w_yaw_max);
    RCLCPP_INFO(logger_, "  delta_t: %.2f, num_samples: %d", params_.delta_t, params_.num_samples);
}

//! HOLONOMIC ELLIPSOIDAL SAMPLE GENERATOR

HolonomicEllipsoidalSampleGenerator::HolonomicEllipsoidalSampleGenerator(const rclcpp::Logger& logger)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing holonomic ellipsoidal sample generator with default parameters");
    // Default parameters already initialized in MotionParameters struct
}

HolonomicEllipsoidalSampleGenerator::HolonomicEllipsoidalSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing holonomic ellipsoidal sample generator");
    // Parameters will be set by the AVOA node
}

void HolonomicEllipsoidalSampleGenerator::setMotionParameters(const MotionParameters& params)
{
    params_ = params;
    
    RCLCPP_INFO(logger_, "Set holonomic ellipsoidal parameters:");
    RCLCPP_INFO(logger_, "  a_x_max: %.2f", params_.a_x_max);
    RCLCPP_INFO(logger_, "  a_y_max: %.2f", params_.a_y_max);
    RCLCPP_INFO(logger_, "  a_z_max: %.2f", params_.a_z_max);
    RCLCPP_INFO(logger_, "  v_x_max: %.2f", params_.v_x_max);
    RCLCPP_INFO(logger_, "  v_y_max: %.2f", params_.v_y_max);
    RCLCPP_INFO(logger_, "  v_z_max: %.2f", params_.v_z_max);
    RCLCPP_INFO(logger_, "  delta_t: %.2f", params_.delta_t);
    RCLCPP_INFO(logger_, "  num_samples: %d", params_.num_samples);
}

bool HolonomicEllipsoidalSampleGenerator::isInsideEllipsoid(double vx, double vy, double vz, 
                                                           double vx_max, double vy_max, double vz_max) const
{
    // Check if point is inside ellipsoid: (vx/vx_max)² + (vy/vy_max)² + (vz/vz_max)² <= 1
    double normalized_sum = (vx * vx) / (vx_max * vx_max) + 
                           (vy * vy) / (vy_max * vy_max) + 
                           (vz * vz) / (vz_max * vz_max);
    return normalized_sum <= 1.0;
}

VelocitySample HolonomicEllipsoidalSampleGenerator::generateEllipsoidSample(std::mt19937& gen,
                                                                           double vx_max, double vy_max, double vz_max) const
{
    // Generate uniform samples within ellipsoid using spherical coordinates
    std::uniform_real_distribution<> dist_r(0.0, 1.0);
    std::uniform_real_distribution<> dist_theta(0.0, 2.0 * M_PI);
    std::uniform_real_distribution<> dist_cos_phi(-1.0, 1.0);
    
    // Generate radius with cube root for uniform volume distribution
    double r = std::cbrt(dist_r(gen));
    double theta = dist_theta(gen);
    double cos_phi = dist_cos_phi(gen);
    double sin_phi = std::sqrt(1.0 - cos_phi * cos_phi);
    
    // Convert to Cartesian coordinates and scale by ellipsoid semi-axes
    double vx = r * vx_max * sin_phi * std::cos(theta);
    double vy = r * vy_max * sin_phi * std::sin(theta);
    double vz = r * vz_max * cos_phi;
    
    return VelocitySample(vx, vy, vz);
}

std::vector<VelocitySample> HolonomicEllipsoidalSampleGenerator::generateSamples(
    const geometry_msgs::msg::Twist& current_velocity,
    const geometry_msgs::msg::Twist& desired_velocity)
{
    std::vector<VelocitySample> samples;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Calculate velocity limits based on current velocity and acceleration constraints
    // This creates a "reachable ellipsoid" that's potentially smaller than the full velocity ellipsoid
    
    //! Calculate reachable velocity bounds (same as original holonomic) from the acceleration constraints
    double min_vx = std::max(current_velocity.linear.x - params_.a_x_max * params_.delta_t, -params_.v_x_max);
    double max_vx = std::min(current_velocity.linear.x + params_.a_x_max * params_.delta_t, params_.v_x_max);
    
    double min_vy = std::max(current_velocity.linear.y - params_.a_y_max * params_.delta_t, -params_.v_y_max);  
    double max_vy = std::min(current_velocity.linear.y + params_.a_y_max * params_.delta_t, params_.v_y_max);
    
    double min_vz = std::max(current_velocity.linear.z - params_.a_z_max * params_.delta_t, -params_.v_z_max);
    double max_vz = std::min(current_velocity.linear.z + params_.a_z_max * params_.delta_t, params_.v_z_max);
    
    std::uniform_real_distribution<> dist_vx(min_vx, max_vx);
    std::uniform_real_distribution<> dist_vy(min_vy, max_vy);
    std::uniform_real_distribution<> dist_vz(min_vz, max_vz);
    
    int attempts = 0;
    int max_attempts = params_.num_samples * 10; // Prevent infinite loops
    
    //! Generate samples by sampling from the reachable acceleration box, 
    // and crop/reject those outside the global velocity ellipsoid.
    while (samples.size() < static_cast<size_t>(params_.num_samples) && attempts < max_attempts) {
        double vx = dist_vx(gen);
        double vy = dist_vy(gen);
        double vz = dist_vz(gen);
        
        if (isInsideEllipsoid(vx, vy, vz, params_.v_x_max, params_.v_y_max, params_.v_z_max)) {
            samples.push_back(VelocitySample(vx, vy, vz));
        }
        attempts++;
    }
    
    // Fallback: None right now
    
    //! Always include the current velocity as a sample (if valid)
    if (current_velocity.linear.x >= min_vx && current_velocity.linear.x <= max_vx &&
        current_velocity.linear.y >= min_vy && current_velocity.linear.y <= max_vy &&
        current_velocity.linear.z >= min_vz && current_velocity.linear.z <= max_vz &&
        isInsideEllipsoid(current_velocity.linear.x, current_velocity.linear.y, current_velocity.linear.z,
                         params_.v_x_max, params_.v_y_max, params_.v_z_max)) {
        samples.push_back(VelocitySample(
            current_velocity.linear.x,
            current_velocity.linear.y,
            current_velocity.linear.z
        ));
    }
    
    //! Always include the desired velocity as a sample (if valid)
    if (desired_velocity.linear.x >= min_vx && desired_velocity.linear.x <= max_vx &&
        desired_velocity.linear.y >= min_vy && desired_velocity.linear.y <= max_vy &&
        desired_velocity.linear.z >= min_vz && desired_velocity.linear.z <= max_vz &&
        isInsideEllipsoid(desired_velocity.linear.x, desired_velocity.linear.y, desired_velocity.linear.z,
                         params_.v_x_max, params_.v_y_max, params_.v_z_max)) {
        samples.push_back(VelocitySample(
            desired_velocity.linear.x,
            desired_velocity.linear.y,
            desired_velocity.linear.z
        ));
    }
    
    return samples;
}

geometry_msgs::msg::Twist HolonomicEllipsoidalSampleGenerator::translateToTwist(const VelocitySample& sample)
{
    // Return as is (same as regular holonomic)
    geometry_msgs::msg::Twist twist;
    twist.linear.x = sample.vx;
    twist.linear.y = sample.vy;
    twist.linear.z = sample.vz;
    twist.angular.x = 0.0;
    twist.angular.y = 0.0;
    twist.angular.z = 0.0;
    return twist;
}

void HolonomicEllipsoidalSampleGenerator::setParams(
    double v_x_max, double v_y_max, double v_z_max,
    double a_x_max, double a_y_max, double a_z_max,
    double w_roll_max, double w_pitch_max, double w_yaw_max,
    double a_roll_max, double a_pitch_max, double a_yaw_max,
    double delta_t, int num_samples) 
{
    // Store parameters in the params_ structure
    params_.v_x_max = v_x_max;
    params_.v_y_max = v_y_max;
    params_.v_z_max = v_z_max;
    params_.a_x_max = a_x_max;
    params_.a_y_max = a_y_max;
    params_.a_z_max = a_z_max;
    params_.w_roll_max = w_roll_max;
    params_.w_pitch_max = w_pitch_max;
    params_.w_yaw_max = w_yaw_max;
    params_.a_roll_max = a_roll_max;
    params_.a_pitch_max = a_pitch_max;
    params_.a_yaw_max = a_yaw_max;
    params_.delta_t = delta_t;
    params_.num_samples = num_samples;
    
    // Log parameters
    RCLCPP_INFO(logger_, "Set holonomic ellipsoidal parameters:");
    RCLCPP_INFO(logger_, "  Linear velocity limits [X,Y,Z]: [%.2f, %.2f, %.2f]", 
               params_.v_x_max, params_.v_y_max, params_.v_z_max);
    RCLCPP_INFO(logger_, "  Linear accel limits [X,Y,Z]: [%.2f, %.2f, %.2f]", 
               params_.a_x_max, params_.a_y_max, params_.a_z_max);
    RCLCPP_INFO(logger_, "  Angular velocity limits [R,P,Y]: [%.2f, %.2f, %.2f]", 
               params_.w_roll_max, params_.w_pitch_max, params_.w_yaw_max);
    RCLCPP_INFO(logger_, "  delta_t: %.2f, num_samples: %d", params_.delta_t, params_.num_samples);
}

//! Diff Drive Sample Generator

DiffDriveSampleGenerator::DiffDriveSampleGenerator(const rclcpp::Logger& logger)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing diff drive generator with default parameters");
    // Default parameters already initialized in MotionParameters struct
}

DiffDriveSampleGenerator::DiffDriveSampleGenerator(const rclcpp::Logger& logger, const rclcpp::Node* node)
    : logger_(logger),  
      random_engine_(std::random_device()())
{
    RCLCPP_INFO(logger_, "Initializing diff drive generator");
    // Parameters will be set by the AVOA node
}

void DiffDriveSampleGenerator::setMotionParameters(const MotionParameters& params)
{
    params_ = params;
    
    RCLCPP_INFO(logger_, "Set diff drive parameters:");
    RCLCPP_INFO(logger_, "  v_x_max (linear): %.2f", params_.v_x_max);
    RCLCPP_INFO(logger_, "  w_yaw_max (angular): %.2f", params_.w_yaw_max);
    RCLCPP_INFO(logger_, "  a_x_max (linear accel): %.2f", params_.a_x_max);
    RCLCPP_INFO(logger_, "  a_yaw_max (angular accel): %.2f", params_.a_yaw_max);
    RCLCPP_INFO(logger_, "  delta_t: %.2f", params_.delta_t);
    RCLCPP_INFO(logger_, "  num_samples: %d", params_.num_samples);
}

std::vector<VelocitySample> DiffDriveSampleGenerator::generateSamples(
    const geometry_msgs::msg::Twist& current_velocity,
    const geometry_msgs::msg::Twist& desired_velocity)
{
    std::vector<VelocitySample> samples;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Extract current linear and angular velocities
    double current_v = current_velocity.linear.x; 
    double current_w = current_velocity.angular.z;
    
    // Calculate velocity limits based on current velocity and acceleration constraints
    // For differential drive: linear acceleration constraint and angular acceleration constraint
    
    // Linear velocity bounds (forward/backward)
    double min_v = std::max(current_v - params_.a_x_max * params_.delta_t, 0.0); // No negative linear velocity
    double max_v = std::min(current_v + params_.a_x_max * params_.delta_t, params_.v_x_max);
    
    // Angular velocity bounds (turning)
    double min_w = std::max(current_w - params_.a_yaw_max * params_.delta_t, -params_.w_yaw_max);
    double max_w = std::min(current_w + params_.a_yaw_max * params_.delta_t, params_.w_yaw_max);
        
    // Create distributions based on the calculated bounds
    std::uniform_real_distribution<> dist_v(min_v, max_v);
    std::uniform_real_distribution<> dist_w(min_w, max_w);
    
    // Generate random samples
    for (int i = 0; i < params_.num_samples; ++i) {
        double v = dist_v(gen);
        double w = dist_w(gen);
        
        // Convert to vx, vy, vz (your preferred approach)
        double vx = v * std::cos(w * params_.delta_t); // Assuming w is in radians and delta_t is time step
        double vy = v * std::sin(w * params_.delta_t); // Convert to Cartesian coordinates
        double vz = 0.0;
        
        samples.push_back(VelocitySample(vx, vy, vz));
    }
    
    //! Always include the current velocity as a sample (if valid)
    if (current_v >= min_v && current_v <= max_v &&
        current_w >= min_w && current_w <= max_w) {
        double vx = current_v * std::cos(current_w * params_.delta_t);
        double vy = current_v * std::sin(current_w * params_.delta_t);
        samples.push_back(VelocitySample(vx, vy, 0.0));
    }
    
    //! Always include the desired velocity as a sample (if valid and within constraints)
    double desired_v = std::sqrt(desired_velocity.linear.x * desired_velocity.linear.x + 
                                desired_velocity.linear.y * desired_velocity.linear.y);
    double desired_w = std::atan2(desired_velocity.linear.y, desired_velocity.linear.x) / params_.delta_t;
    
    if (desired_v >= min_v && desired_v <= max_v &&
        desired_w >= min_w && desired_w <= max_w) {
        double vx = desired_v * std::cos(desired_w * params_.delta_t);
        double vy = desired_v * std::sin(desired_w * params_.delta_t);
        samples.push_back(VelocitySample(vx, vy, 0.0));
    }
    
    return samples;
}
//!Alterado o sample generator
geometry_msgs::msg::Twist DiffDriveSampleGenerator::translateToTwist(const VelocitySample& sample)
{
    geometry_msgs::msg::Twist twist;
    
    //!Handle zero or near-zero velocities
    if (std::abs(sample.vx) == 0.0 && std::abs(sample.vy) == 0.0) {
        return twist;  // All zeros
    }
    
    double angle;
    //!If x is zero, but y is not, then angle is pi/2 or -pi/2
    if (std::abs(sample.vx) < 1e-6 && std::abs(sample.vy) > 1e-6) {
        angle = (sample.vy > 0) ? M_PI_2 : -M_PI_2;
    }
    
    //!General case 
    else {
        angle = std::atan2(sample.vy, sample.vx);
    }
    
    //!Remap the velocity
    twist.linear.x = std::sqrt(sample.vx * sample.vx + sample.vy * sample.vy);

    twist.angular.z = angle / params_.delta_t;
    
    return twist;
}


double DiffDriveSampleGenerator::normalizeAngle(double angle)
{
    while (angle > M_PI) angle -= 2*M_PI;
    while (angle < -M_PI) angle += 2*M_PI;
    return angle;
}

void DiffDriveSampleGenerator::setParams(
    double v_x_max, double v_y_max, double v_z_max,
    double a_x_max, double a_y_max, double a_z_max,
    double w_roll_max, double w_pitch_max, double w_yaw_max,
    double a_roll_max, double a_pitch_max, double a_yaw_max,
    double delta_t, int num_samples) 
{
    // Store parameters in the params_ structure
    params_.v_x_max = v_x_max;
    params_.v_y_max = v_y_max;
    params_.v_z_max = v_z_max;
    params_.a_x_max = a_x_max;
    params_.a_y_max = a_y_max;
    params_.a_z_max = a_z_max;
    params_.w_roll_max = w_roll_max;
    params_.w_pitch_max = w_pitch_max;
    params_.w_yaw_max = w_yaw_max;
    params_.a_roll_max = a_roll_max;
    params_.a_pitch_max = a_pitch_max;
    params_.a_yaw_max = a_yaw_max;
    params_.delta_t = delta_t;
    params_.num_samples = num_samples;
    
    // Log parameters
    RCLCPP_INFO(logger_, "Set diff drive parameters:");
    RCLCPP_INFO(logger_, "  Linear velocity limits [X,Y,Z]: [%.2f, %.2f, %.2f]", 
               params_.v_x_max, params_.v_y_max, params_.v_z_max);
    RCLCPP_INFO(logger_, "  Linear accel limits [X,Y,Z]: [%.2f, %.2f, %.2f]", 
               params_.a_x_max, params_.a_y_max, params_.a_z_max);
    RCLCPP_INFO(logger_, "  Angular velocity limits [R,P,Y]: [%.2f, %.2f, %.2f]", 
               params_.w_roll_max, params_.w_pitch_max, params_.w_yaw_max);
    RCLCPP_INFO(logger_, "  delta_t: %.2f, num_samples: %d", params_.delta_t, params_.num_samples);
}

} // namespace avoa3d