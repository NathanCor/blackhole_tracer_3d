#include <iostream>
#include <vector>
#include <cmath>
#include <filesystem>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <omp.h>

#include "../include/Vec3.hpp"
#include "../include/AccretionDisk.hpp"
#include "../include/Camera.hpp"
#include "../include/Dither.hpp"
#include <rk4.hpp>

constexpr int WIDTH = 1920;
constexpr int HEIGHT = 1080;
constexpr int NUM_FRAMES = 120;
constexpr double ORBIT_RADIUS = 36.0;
constexpr double CAM_ALTITUDE = 6.2;
constexpr double FOV = 0.58;
constexpr double TILT_ANGLE = -0.38;
constexpr double M = 1.0;
constexpr double RS = 2.0 * M;
constexpr double DS = 0.09;

struct RayState {
    Vec3 pos;
    Vec3 vel;
    RayState operator+(const RayState& other) const { return {pos + other.pos, vel + other.vel}; }
    RayState operator*(double scalar) const { return {pos * scalar, vel * scalar}; }
};

RayState computeDerivative(double /*t*/, const RayState& state) {
    double r = state.pos.norm();
    if (r <= RS) return {state.vel, Vec3(0, 0, 0)};
    double factor = -M / ((r - RS) * (r - RS) * r);
    return {state.vel, state.pos * factor};
}

double sampleNebula(const Vec3& vel) {
    Vec3 dir = vel.normalized();
    double noise = 0.0;
    noise += std::sin(dir.x * 5.0) * std::cos(dir.y * 4.0) * std::sin(dir.z * 5.0);
    noise += 0.5 * std::sin(dir.x * 10.0 + 2.0) * std::cos(dir.y * 9.0 - 1.0) * std::sin(dir.z * 11.0 + 3.0);
    noise += 0.25 * std::sin(dir.x * 20.0 - 1.0) * std::cos(dir.y * 18.0 + 4.0) * std::sin(dir.z * 21.0 - 2.0);

    noise = (noise / 1.75) * 0.5 + 0.5;

    if (noise < 0.65) return 0.0;
    return (noise - 0.65) * 0.3;
}

int main() {
    std::filesystem::create_directory("output");
    AccretionDisk disk(5.0, 19.5, M);

    std::cout << "Génération de la boucle parfaite avec nébuleuse (" << NUM_FRAMES << " frames)...\n";

    for (int frame = 0; frame < NUM_FRAMES; ++frame) {
        std::vector<float> luminanceBuffer(WIDTH * HEIGHT, 0.0f);

        double time_fraction = static_cast<double>(frame) / NUM_FRAMES;
        double theta = 2.0 * M_PI * time_fraction;

        Vec3 cam_pos(ORBIT_RADIUS * std::sin(theta), CAM_ALTITUDE, ORBIT_RADIUS * std::cos(theta));

        Camera cam(cam_pos, Vec3(0.0, 0.0, 0.0), Vec3(0.0, 1.0, 0.0), FOV,
                   static_cast<double>(WIDTH) / HEIGHT, TILT_ANGLE);

        #pragma omp parallel for schedule(dynamic, 8)
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                double nx = 2.0 * (x + 0.5) / WIDTH - 1.0;
                double ny = 1.0 - 2.0 * (y + 0.5) / HEIGHT;

                RayState state = {cam.getPosition(), cam.getRayDirection(nx, ny)};
                double accumulatedIntensity = 0.0;

                for (int step = 0; step < 1100; ++step) {
                    double r = state.pos.norm();

                    if (r <= RS * 1.008) break;

                    if (r > 48.0) {
                        accumulatedIntensity += sampleNebula(state.vel);
                        break;
                    }

                    RayState nextState = nummeth::rk4_step(computeDerivative, 0.0, state, DS);

                    double dI = disk.sample(state.pos, nextState.pos, state.vel, time_fraction);
                    if (dI > 0.0) {
                        accumulatedIntensity += dI;
                    }
                    state = nextState;
                }
                luminanceBuffer[y * WIDTH + x] = static_cast<float>(accumulatedIntensity);
            }
        }

        float maxVal = 0.0f;
        for (float v : luminanceBuffer) {
            if (v > maxVal) maxVal = v;
        }

        if (maxVal > 0.0f) {
            for (float& v : luminanceBuffer) {
                if (v <= 0.0f) continue;
                float norm = v / maxVal;
                float boosted = std::pow(norm, 0.42f) * 1.25f;
                v = std::clamp(boosted, 0.0f, 1.0f);
            }
        }

        std::vector<uint8_t> binaryImage;
        DitherProcessor::applyFloydSteinberg(luminanceBuffer, WIDTH, HEIGHT, binaryImage);

        std::ostringstream filename;
        filename << "output/frame_" << std::setw(4) << std::setfill('0') << frame << ".ppm";
        DitherProcessor::writePPM(filename.str(), binaryImage, WIDTH, HEIGHT);

        std::cout << "\rFrame " << frame + 1 << "/" << NUM_FRAMES << std::flush;
    }

    std::cout << "\nEncodage : ffmpeg -framerate 30 -i output/frame_%04d.ppm -c:v libx264 -pix_fmt yuv420p blackhole_loop_nebula.mp4\n";
    return 0;
}