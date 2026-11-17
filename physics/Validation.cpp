#include <numbers>

#include "Validation.hpp"

ValidationResult Validation_Config(SimulationConfig& c)
{
    if (c.force_N <= 0.0) return {false, "Force must be greater than zero"};
    if (c.impulseTime_s <= 0) return {false, "Impulse time must be greater than zero"};
    if (c.mass_kg <= 0.0) return {false, "Mass must be greater than zero"};
    if (c.angle_rad <= 0.0 || c.angle_rad >= std::numbers::pi / 2) return {false, "Muzzle angle must be between 0 and 90 degrees"};
    if (c.gravity <= 0.0) return {false, "Gravity must be greater than zero"};
    if (c.muzzle.x < 0 || c.muzzle.y < 0) return {false, "Muzzle must be above the ground"};
    if (c.timeStep_s <= 0.0 || c.timeStep_s > 0.1) return {false, "Time step must be greater than 0 and less than 0.1s"};
    if (c.maxTime_s <= 0.0) return {false, "Max time must be greater than zero"};
    return {};
}

ValidationResult Validation_NextStep(ProjectileState& state, SimulationConfig& config)
{
    if (state.position.y <= 0) { return {false, "Ball hit the ground"}; }
    return {};
}