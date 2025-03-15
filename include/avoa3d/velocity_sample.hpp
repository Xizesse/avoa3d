
#ifndef AVOA3D_VELOCITY_SAMPLE_HPP
#define AVOA3D_VELOCITY_SAMPLE_HPP
namespace avoa3d {

// Simple structure to represent a velocity sample
struct VelocitySample {
    double vx;
    double vy;
    double vz;
    double cost;
    
    VelocitySample(double _vx, double _vy, double _vz) 
        : vx(_vx), vy(_vy), vz(_vz), cost(999) {}
};

} 

#endif 