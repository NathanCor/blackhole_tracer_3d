cmake_minimum_required(VERSION 3.20)
project(blackhole_tracer_3d CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimisations du compilateur
if (MSVC)
    add_compile_options(/O2 /fp:fast /openmp)
else()
    add_compile_options(-O3 -ffast-math -fopenmp)
endif()

# Téléchargement et intégration du package rk4_integrator
include(FetchContent)
FetchContent_Declare(
        rk4_integrator
        GIT_REPOSITORY https://github.com/NathanCor/rk4_integrator.git
        GIT_TAG        main
)
FetchContent_MakeAvailable(rk4_integrator)

# Fichiers sources du projet
add_executable(${PROJECT_NAME}
        src/main.cpp
        src/AccretionDisk.cpp
        src/Dither.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        include
        ${rk4_integrator_SOURCE_DIR}/include
)

# Lien avec le package rk4_integrator et OpenMP
find_package(OpenMP REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
        rk4_integrator
        OpenMP::OpenMP_CXX
)