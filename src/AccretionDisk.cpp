#include "../include/AccretionDisk.hpp"
#include <algorithm>
#include <cmath>

double AccretionDisk::sample(const Vec3& pOld, const Vec3& pNew, const Vec3& rayDir, double time_fraction) const {
    if ((pOld.y > 0.0 && pNew.y > 0.0) || (pOld.y < 0.0 && pNew.y < 0.0)) {
        return 0.0;
    }

    double t = -pOld.y / (pNew.y - pOld.y);
    Vec3 hit = pOld + (pNew - pOld) * t;
    double r = std::sqrt(hit.x * hit.x + hit.z * hit.z);

    if (r < rIn || r > rOut) return 0.0;

    double phi = std::atan2(hit.z, hit.x);
    double omega = std::sqrt(M / (r * r * r));

    constexpr double LOOP_DURATION = 150.0;
    double t1 = time_fraction * LOOP_DURATION;
    double t2 = t1 - LOOP_DURATION;

    auto getTurbulence = [&](double time_val) {
        double phi_anim = phi - omega * time_val;
        double turb = 1.0
                    + 0.45 * std::sin(5.0 * phi_anim)
                    + 0.25 * std::sin(12.0 * phi_anim - r * 1.5);
        return std::max(0.1, turb);
    };

    double turb1 = getTurbulence(t1);
    double turb2 = getTurbulence(t2);
    double turbulence = turb1 * (1.0 - time_fraction) + turb2 * time_fraction;

    double core = std::exp(-std::pow((r - 6.1) / 1.3, 2.0)) * 3.6;
    double ring1 = std::exp(-std::pow((r - 8.6) / 0.85, 2.0)) * 1.5;
    double ring2 = std::exp(-std::pow((r - 11.4) / 1.1, 2.0)) * 1.1;
    double ring3 = std::exp(-std::pow((r - 14.8) / 1.6, 2.0)) * 0.7;
    double tail = std::exp(-(r - rIn) / 4.2) * 0.5;

    double baseProfile = (core + ring1 + ring2 + ring3 + tail) * turbulence;

    double v_phi = std::sqrt(M / r);
    Vec3 diskVel{-hit.z / r * v_phi, 0.0, hit.x / r * v_phi};
    Vec3 k = rayDir.normalized() * -1.0;

    double gamma = 1.0 / std::sqrt(std::max(0.02, 1.0 - diskVel.normSq()));
    double dop = 1.0 / (gamma * (1.0 - diskVel.dot(k)));
    double red = std::sqrt(std::max(0.02, 1.0 - (2.0 * M) / r));
    double g = dop * red;

    return baseProfile * std::pow(g, 1.35);
}