#include <stdexcept>
#include <iostream>

#include "Simulation.hpp"
#include "PhysicsCalculations.hpp"
#include "ProjectileState.hpp"
#include "SimulationConfig.hpp"
#include "Validation.hpp"
#include "SaveResults.hpp"

void Simulation::AddStep(const ProjectileState& state_)
{
    auto newStack = new StepsStack{state_};

    tail->next = newStack;
    tail = newStack;
    count++;
}

Simulation::Simulation()
{
    config = SimulationConfig();
    stack.state = initialState(config);
}
Simulation::Simulation(SimulationConfig config_)
{
    auto validation = Validation_Config(config_);
    if (!validation) throw std::runtime_error(validation.errorMessage);

    config = config_;
    stack.state = initialState(config);
}
Simulation::~Simulation()
{
    auto current = stack.next;
    while (current)
    {
        auto next = current->next;
        delete current;
        current = next;
    }
}

ValidationResult Simulation::Run()
{
    if (SimulationFinished) throw std::runtime_error("Simulation has already been run");

    ProjectileState currentStep = stack.state;
    ProjectileState nextStep;
    ValidationResult validation_result;
    SimulationFinished = true;

    while (currentStep.time < config.maxTime_s)
    {
        nextStep = eulerStep(config, currentStep);
        // w systemie dwójkowym 0.001 nie da się zapisać wymiernie więc count mnożymy przez krok aby zachować doładność
        nextStep.time = static_cast<double>(count) * config.timeStep_s;
        AddStep(nextStep);

        validation_result = Validation_NextStep(nextStep, config);
        if (!validation_result)
        {
            tail->state = lastStepCorrection(currentStep, nextStep);

            auto fileName = getFileName();
            if (auto result = SaveBin(&stack, count, fileName); !result) { std::cerr << result.errorMessage << std::endl; }
            if (auto result = SaveCsv(&stack, fileName); !result) { std::cerr << result.errorMessage << std::endl; }

            return {true, validation_result.errorMessage};
        }
        currentStep = nextStep;
    }
    return {false, "Projectile exceeded maximum time in air"};
}

StepsStack Simulation::getResult() const
{
    if (!SimulationFinished) throw std::runtime_error("Simulation has not yet been ran");
    return stack;
}

int Simulation::getCount() const
{
    if (!SimulationFinished) throw std::runtime_error("Simulation has not yet been ran");
    return count;
}
