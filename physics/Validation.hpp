#pragma once
#include <iostream>

#include "SimulationConfig.hpp"
#include "ProjectileState.hpp"

struct ValidationResult
{
    bool statusOk = true;
    std::string errorMessage;

    explicit operator bool() const { return statusOk; }
};

ValidationResult Validation_Config(SimulationConfig& config);
ValidationResult Validation_NextStep(ProjectileState& state, SimulationConfig& config);