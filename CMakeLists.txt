cmake_minimum_required(VERSION 3.20)
project(blackhole_tracer_3d CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Importation du package RK4 depuis GitHub
include(FetchContent)
FetchContent_Declare(
    rk4_integrator
    GIT_REPOSITORY https://github.com/NathanCor/rk4_integrator.git
    GIT_TAG        main 
)
FetchContent_MakeAvailable(rk4_integrator)

# Définition de l'exécutable principal
add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Édite "rk4_integrator" si le nom de la bibliothèque exportée dans ton dépôt est différent
target_link_libraries(${PROJECT_NAME} PRIVATE rk4_integrator)
