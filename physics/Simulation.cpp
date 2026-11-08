#pragma once
#include <stdexcept>

#include "PhysicsCalculations.hpp"
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

    void AddStep(const ProjectileState& state_)
    {
        auto newStack = new StepsStack{state_};

        tail->next = newStack;
        tail = newStack;
        count++;
    }

public:
    Simulation()
    {
        config = SimulationConfig();
        stack.state = initialState(config);
    }
    Simulation(SimulationConfig config_)
    {
        auto validation = Validation_Config(config_);
        if (!validation) throw std::runtime_error(validation.errorMessage);

        config = config_;
        stack.state = initialState(config);
    }

    ValidationResult Run()
    {
        if (SimulationFinished) throw std::runtime_error("Simulation has already been run");

        ProjectileState currentStep = stack.state;
        ValidationResult validation_result;
        SimulationFinished = true;
        for (double time = 0; time <= config.maxTime_s; time += config.timeStep_s)
        {
            currentStep = eurelStep(config, currentStep);
            AddStep(currentStep);

            validation_result = Validation_NextStep(currentStep, config);
            if (!validation_result) return {true, validation_result.errorMessage};
        }
        return {false, "Projectile exceeded maximum time in air"};
    }

    StepsStack getResult() const
    {
        if (!SimulationFinished) throw std::runtime_error("Simulation has not yet been ran");
        return stack;
    }

    int getCount() const
    {
        if (!SimulationFinished) throw std::runtime_error("Simulation has not yet been ran");
        return count;
    }
};
