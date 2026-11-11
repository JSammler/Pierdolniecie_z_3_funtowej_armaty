#pragma once

#include "SimulationConfig.hpp"
#include "ProjectileState.hpp"

double impulse(const SimulationConfig& config);
double initialSpeed(const SimulationConfig& config);
Vec2 initialVelocity(const SimulationConfig& config);

Vec2 force(const SimulationConfig& config, const ProjectileState& state);
Vec2 acceleration(const SimulationConfig& config, const ProjectileState& state);

ProjectileState initialState(const SimulationConfig& config);
ProjectileState eulerStep(const SimulationConfig& config, const ProjectileState& state);

ProjectileState lastStepCorrection(const ProjectileState& stateA, const ProjectileState& stateB);