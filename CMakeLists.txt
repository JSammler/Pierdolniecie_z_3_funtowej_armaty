cmake_minimum_required(VERSION 4.3.1)
project(Pierdolniecie_z_3_funtowej_armaty)

set(CMAKE_CXX_STANDARD 23)

include_directories(physics)

# Moduł fizyki (bez zmian w kodzie)
add_library(physics STATIC
        physics/PhysicsCalculations.cpp
        physics/PhysicsCalculations.hpp
        physics/ProjectileState.hpp
        physics/Simulation.cpp
        physics/Simulation.hpp
        physics/SimulationConfig.hpp
        physics/Validation.cpp
        physics/Validation.hpp
        physics/Vec2.hpp)
target_include_directories(physics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/physics)

# GUI: OpenGL + ImGui
add_subdirectory(gui)

add_executable(Pierdolniecie_z_3_funtowej_armaty main.cpp)
target_link_libraries(Pierdolniecie_z_3_funtowej_armaty PRIVATE gui physics)
