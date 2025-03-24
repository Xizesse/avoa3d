
#ifndef AVOA3D_VELOCITY_SAMPLE_HPP
#define AVOA3D_VELOCITY_SAMPLE_HPP
namespace avoa3d {

// Simple structure to represent a velocity sample
struct VelocitySample {
    double vx;
    double vy;
    double vz;
    double cost;
    double danger;

    VelocitySample() : vx(0.0), vy(0.0), vz(0.0), cost(999), danger(0) {}
    
    VelocitySample(double _vx, double _vy, double _vz) 
        : vx(_vx), vy(_vy), vz(_vz), cost(999), danger(0) {}
};

} 

#endif 