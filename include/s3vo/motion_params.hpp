#ifndef S3VO_MOTION_PARAMS_HPP
#define S3VO_MOTION_PARAMS_HPP

namespace s3vo
{

// Common structure to hold all motion parameters
struct MotionParameters 
{
    // Kinematic mode
    std::string kinematic_mode = "holonomic"; // Options: "holonomic", "diff_drive"
    double vehicle_radius = 0.5; // Vehicle radius in meters

    //Weights
    double heading_weight = 0.5;
    double danger_weight = 0.5;
    double abs_weight = 0.5;

    // Maximum linear accelerations
    double a_x_max = 3.0;
    double a_y_max = 3.0;
    double a_z_max = 3.0;
    
    // Maximum angular accelerations
    double a_roll_max = 0.0;
    double a_pitch_max = 0.0;
    double a_yaw_max = 0.0;
    
    // Maximum linear velocities
    double v_x_max = 1.0;
    double v_y_max = 1.0;
    double v_z_max = 0.0;
    
    // Maximum angular velocities
    double w_roll_max = 0.0;
    double w_pitch_max = 0.0;
    double w_yaw_max = 1.0;
    
    // Time step
    double delta_t = 1.0;
    
    // Number of samples
    int num_samples = 10000;
};

} // namespace s3vo

#endif // S3VO_MOTION_PARAMS_HPP