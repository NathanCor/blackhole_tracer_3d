<p align="center">
  <img src="blackhole_dithered.gif" alt="Dithered black hole with an orbiting camera" width="100%">
</p>

# blackhole_tracer_3d

A C++20 ray tracer that renders a stylized black hole with an accretion disk, seen by a camera that makes a full 360° orbit around it. The frames are converted to a two-tone ordered-dither look and assembled into a seamless loop.

This is the third and final part of a trilogy of black hole simulation projects (see below).

---

## The Black Hole Trilogy

This project is the last chapter of a three-part series, going from the physics of a single trajectory to a full animated render:

| # | Project | What it does |
| :-- | :-- | :-- |
| 1 | [`schwarzschild_effective_potential`](https://github.com/NathanCor/schwarzschild_effective_potential) | Integrates the fall of a massive particle in the Schwarzschild metric (effective potential, RK4) and plots its orbit. |
| 2 | [`blackhole_tracer`](https://github.com/NathanCor/blackhole_tracer) | Ray tracer producing a still image of a black hole and its accretion disk from a fixed viewpoint, with a benchmark against the exact null geodesics. |
| 3 | **`blackhole_tracer_3d`** (this repository) | Animated version: the camera orbits the black hole, and the disk rotates in a seamless loop. |

---

## Overview

For each of the 120 frames, the camera moves along a circle of radius $36\,M$ around the black hole, always looking at the center. One ray is traced backward through the gravitational field for every pixel. Rays that fall below the horizon stay black, rays that cross the equatorial plane pick up light from the accretion disk, and rays that escape sample a procedural nebula in their final direction, so the background is bent by the black hole as well.

The accretion disk rotates differentially (inner rings faster than outer rings), and the animation is built so that the last frame connects exactly to the first one.

---

## Physical Model

This is a **stylized approximation**, not a general-relativistic simulation.

### Light bending
Rays follow a Newtonian force derived from the **Paczyński–Wiita pseudo-potential**

$$\Phi(r) = -\frac{M}{r - r_s}, \qquad r_s = 2M, \qquad \vec{a} = -\nabla\Phi$$

integrated with a 4th-order Runge–Kutta scheme (fixed step, provided by the [`rk4_integrator`](https://github.com/NathanCor/rk4_integrator) package). This is the same model as in `blackhole_tracer`, where it is benchmarked against the exact Schwarzschild null geodesics: it reproduces the capture threshold ($b_c \approx 3\sqrt{3}\,M$) and the closest-approach radius well, but it deflects light only about **half** as much as general relativity. The lensing in the animation is therefore qualitatively right and quantitatively too weak.

### Accretion disk
- Thin disk in the equatorial plane, between $5\,M$ and $19.5\,M$.
- The radial brightness profile is **hand-tuned** (a sum of Gaussian rings plus a decaying tail), not derived from a physical emissivity law.
- Rotating structure: azimuthal turbulence advected at the Keplerian angular velocity $\Omega = \sqrt{M/r^3}$.
- Relativistic shading: disk material moves at $v_\phi = \sqrt{M/r}$, and the brightness is scaled by the Doppler and gravitational redshift factor $g$ raised to an **artistic exponent** ($g^{1.35}$, not the physical $g^4$ beaming law).
- The disk is treated as transparent: contributions from every plane crossing along a ray are added, so the front of the disk does not hide its own lensed image.

### Seamless loop
- The camera angle is $\theta = 2\pi\,f/N$ over $N = 120$ frames, so the last frame is followed by the first.
- The disk turbulence is a cross-fade between two copies of the pattern shifted by one full loop duration, which makes the disk state at $f \to 1$ identical to the one at $f = 0$.

---

## Rendering Pipeline

1. Camera on a circular orbit at altitude $6.2\,M$ (about $10°$ above the disk plane), with a slight roll.
2. One RK4 ray per pixel (OpenMP-parallel over image rows), at most 1100 steps.
3. Stop at the horizon cutoff ($r \le 1.008\,r_s$) or when the ray escapes beyond $r = 48\,M$, where the nebula is sampled.
4. Per-frame tone mapping: normalization by the frame maximum, then a power curve.
5. Ordered dithering with a 4×4 Bayer matrix on 2×2 pixel cells, drawn as light dots on a `#0d1117` background.
6. Frames are written as binary PPM files in `output/`.

---

## Project Structure

```text
.
├── CMakeLists.txt
├── include/
│   ├── AccretionDisk.hpp
│   ├── Camera.hpp
│   ├── Dither.hpp
│   ├── Font8x8.hpp
│   └── Vec3.hpp
└── src/
    ├── AccretionDisk.cpp
    ├── Dither.cpp
    └── main.cpp
```

---

## Build & Usage

### Prerequisites
- C++20 compiler (`gcc >= 11` or `clang >= 13`)
- CMake >= 3.20
- OpenMP
- Git and an internet connection at configure time (the `rk4_integrator` package is fetched with CMake `FetchContent`)
- FFmpeg (to assemble the frames into a video or GIF)

### Build

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Render

Run the executable from the directory where `output/` should be created:

```bash
./blackhole_tracer_3d
```

It writes `output/frame_0000.ppm` to `output/frame_0119.ppm`. At the default 1920×1080 resolution this is CPU-intensive; for a quick preview, lower `WIDTH`, `HEIGHT` and `NUM_FRAMES` in `src/main.cpp`.

### Assemble the animation

```bash
ffmpeg -framerate 30 -i output/frame_%04d.ppm -c:v libx264 -pix_fmt yuv420p blackhole_loop_nebula.mp4
```

For a lighter GIF suited to a README (half resolution, 32-color palette, dither pattern preserved):

```bash
ffmpeg -framerate 30 -i output/frame_%04d.ppm \
  -vf "scale=960:-1:flags=neighbor,split[s0][s1];[s0]palettegen=max_colors=32[p];[s1][p]paletteuse=dither=none" \
  blackhole_dithered.gif
```

---

## Configuration

Camera, resolution and integration parameters are constants at the top of `src/main.cpp`:

```cpp
constexpr int WIDTH = 1920;
constexpr int HEIGHT = 1080;
constexpr int NUM_FRAMES = 120;
constexpr double ORBIT_RADIUS = 36.0;
constexpr double CAM_ALTITUDE = 6.2;
constexpr double FOV = 0.58;
constexpr double TILT_ANGLE = -0.38;
constexpr double DS = 0.09;
```

The disk radii are set where the `AccretionDisk` is constructed in `main()`.

---

## License

This project is open-source under the MIT License.
