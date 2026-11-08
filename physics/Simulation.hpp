#pragma once

#include "ProjectileState.hpp"
#include "SimulationConfig.hpp"
#include "Validation.hpp"

struct StepsStack
{
    ProjectileState state;
    StepsStack* next = nullptr;
};

class Simulation
{
    StepsStack stack{};
    int count{1};
    SimulationConfig config;
    StepsStack* tail = &stack;

    bool SimulationFinished{false};

    void AddStep(const ProjectileState& state_);

public:
    Simulation();
    Simulation(SimulationConfig config_);
    ~Simulation();

    Simulation(const Simulation&) = delete;
    Simulation& operator=(const Simulation&) = delete;

    ValidationResult Run();

    StepsStack getResult() const;
    int getCount() const;
};