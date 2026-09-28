#pragma once
#include "Vec3.hpp"

class AccretionDisk {
public:
    AccretionDisk(double r_in = 6.0, double r_out = 24.0, double mass = 1.0)
        : rIn(r_in), rOut(r_out), M(mass) {}

    double sample(const Vec3& pOld, const Vec3& pNew, const Vec3& rayDir, double time_fraction = 0.0) const;

private:
    double rIn;
    double rOut;
    double M;
};