
#ifndef AVOA3D_VELOCITY_SAMPLE_HPP
#define AVOA3D_VELOCITY_SAMPLE_HPP

namespace avoa3d {

/**
 * @brief Represents a single candidate velocity for the robot.
 */
struct VelocitySample {
    
    // --- 1. EVALUATION DOMAIN (VO Space) ---
    struct {
        double x, y, z;
    } velocity_space;

    // --- 2. CONTROL DOMAIN (Twist Space) ---
    struct {
        double lx, ly, lz; // Linear
        double ax, ay, az; // Angular
    } twist;

    // --- 3. COST BREAKDOWN ---
    double cost;            // Final combined cost
    double danger;          // Safety penalty
    double heading;         // Alignment with goal direction (normalized dot)
    double momentum;        // Penalty for direction changes
    double magnitude;       // Scalar speed error
    double angular;         // Angular mismatch
    double linear_matching; // Vector distance in Evaluation Space
    double twist_matching;  // Normalized error in Control Space

    // Default constructor
    VelocitySample() : cost(999.0), danger(0.0), heading(0.0), momentum(0.0), 
                      magnitude(0.0), angular(0.0), linear_matching(0.0), twist_matching(0.0) {
        velocity_space = {0.0, 0.0, 0.0};
        twist.lx = twist.ly = twist.lz = 0.0;
        twist.ax = twist.ay = twist.az = 0.0;
    }
    
    // Convenience constructor
    VelocitySample(double vx, double vy, double vz, double lx, double ly, double lz, double ax, double ay, double az) 
        : cost(999.0), danger(0.0), heading(0.0), momentum(0.0), 
          magnitude(0.0), angular(0.0), linear_matching(0.0), twist_matching(0.0) 
    {
        velocity_space = {vx, vy, vz};
        twist.lx = lx; twist.ly = ly; twist.lz = lz;
        twist.ax = ax; twist.ay = ay; twist.az = az;
    }
};

} 

#endif 