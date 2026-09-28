#pragma once
#include "Vec3.hpp"
#include <cmath>

class Camera {
private:
    Vec3 pos;
    Vec3 forward;
    Vec3 right;
    Vec3 up;
    double fov_scale;
    double aspect_ratio;

public:
    Camera(const Vec3& position, const Vec3& target, const Vec3& world_up,
           double fov_radians, double aspect, double roll = 0.0)
        : pos(position), aspect_ratio(aspect)
    {
        fov_scale = std::tan(fov_radians * 0.5);
        forward = (target - pos).normalized();

        Vec3 raw_right = forward.cross(world_up).normalized();
        Vec3 raw_up = raw_right.cross(forward).normalized();

        right = raw_right * std::cos(roll) + raw_up * std::sin(roll);
        up = raw_right * (-std::sin(roll)) + raw_up * std::cos(roll);
    }

    Vec3 getPosition() const { return pos; }

    Vec3 getRayDirection(double nx, double ny) const {
        double u = nx * aspect_ratio * fov_scale;
        double v = ny * fov_scale;
        return (forward + right * u + up * v).normalized();
    }
};