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
    return {0.0, -config.gravity * config.mass_kg};
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

ProjectileState eulerStep(const SimulationConfig& config, const ProjectileState& state)
{
    const double dTime = config.timeStep_s;
    ProjectileState result;
    result.time = state.time + dTime; // zbędne przy korzystaniu z Simulation::Run() ale dla spójności zostawione
    result.position = state.position + state.velocity * dTime + state.acceleration * (0.5 * dTime * dTime);
    result.velocity = state.velocity + state.acceleration * dTime;
    result.acceleration = acceleration(config, result);
    return result;
}

ProjectileState lastStepCorrection(const ProjectileState& stateA, const ProjectileState& stateB)
{
    const double f = stateA.position.y / (stateA.position.y - stateB.position.y);

    ProjectileState result;
    result.time = stateA.time + f * (stateB.time - stateA.time);
    result.position = { stateA.position.x + f * (stateB.position.x - stateA.position.x), 0.0 };
    result.velocity = stateA.velocity + (stateB.velocity - stateA.velocity) * f;
    result.acceleration = stateB.acceleration;

    return result;
}