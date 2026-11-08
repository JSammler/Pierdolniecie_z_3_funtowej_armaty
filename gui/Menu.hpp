#pragma once

#include "Camera.hpp"
#include "Game.hpp"

#include "SimulationConfig.hpp"

namespace gui
{

constexpr float kMenuWidth = 390.0f;

// Panel boczny: sterowanie armatą, wszystkie wartości z SimulationConfig, presety, ustawienia gry.
void drawMenu(Game& game, Camera& camera, const SimulationConfig& defaults, float windowHeight);

} // namespace gui
