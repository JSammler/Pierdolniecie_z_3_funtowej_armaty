cmake_minimum_required(VERSION 4.4)
project(Pierdolniecie_z_3_funtowej_armaty)

set(CMAKE_CXX_STANDARD 23)

include_directories(physics)

add_subdirectory(gui)

add_executable(Pierdolniecie_z_3_funtowej_armaty
        physics/PhysicsCalculations.cpp
        physics/PhysicsCalculations.hpp
        physics/ProjectileState.hpp
        physics/Simulation.cpp
        physics/Simulation.hpp
        physics/SimulationConfig.hpp
        physics/Validation.cpp
        physics/Validation.hpp
        physics/Vec2.hpp
        main.cpp)

target_link_libraries(Pierdolniecie_z_3_funtowej_armaty PRIVATE gui)
