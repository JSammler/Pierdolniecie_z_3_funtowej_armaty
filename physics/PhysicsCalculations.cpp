#include <numbers>
#include <cmath>

#include "PhysicsCalculations.hpp"
#include "SimulationConfig.hpp"
#include "ProjectileState.hpp"

double impulse(const SimulationConfig& config)
{
    return config.force_N * config.impulseTime_s;
}
double initialSpeed(const SimulationConfig& config)
{
    return impulse(config) / config.mass_kg;
}
Vec2 initialVelocity(const SimulationConfig& config)
{
    return {initialSpeed(config)*std::cos(config.angle_rad), initialSpeed(config)*std::sin(config.angle_rad)};
}

Vec2 force(const SimulationConfig& config, const ProjectileState& state)
{
    return {0.0, -config.gravity};
}
Vec2 acceleration(const SimulationConfig& config, const ProjectileState& state)
{
    return force(config, state) / config.mass_kg;
}

ProjectileState initialState(const SimulationConfig& config)
{
    ProjectileState result;
    result.time = 0.0;
    result.position = config.muzzle;
    result.velocity = initialVelocity(config);
    result.acceleration = acceleration(config, result);
    return result;
}

ProjectileState eurelStep(const SimulationConfig& config, const ProjectileState& state)
{
    const double dTime = config.timeStep_s;
    ProjectileState result;
    result.time = state.time + dTime;
    result.position = state.position + state.velocity * dTime;
    result.velocity = state.velocity + state.acceleration * dTime;
    result.acceleration = acceleration(config, result);
    return result;
}
