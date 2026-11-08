#pragma once
#include <numbers>

#include "Vec2.hpp"

// Domyślne wartości parametrów
// nazwaParametru_jednostka
struct SimulationConfig
{
    double force_N = 5000.0;
    double impulseTime_s = 0.01;
    double mass_kg = 5.0;
    double angle_rad = std::numbers::pi / 4; // 45deg
    double gravity = 9.80665;
    Vec2 muzzle = {0.0, 0.0}; // lufa względem P(0, 0) na układzie współżędnych
    double timeStep_s = 0.001;
    double maxTime_s = 60.0; // bezpiecznik: najdłuższy lot przed przerwaniem pętli
};