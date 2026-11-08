#pragma once

#include <string>

#include "../physics/SimulationConfig.hpp"
#include "TrajectoryView.hpp"

enum class PanelAction
{
    None,
    Fire,
    Clear,
};

struct StatusMessage
{
    std::string text;
    bool error = false;
};

// Lewy panel: edycja SimulationConfig, ustawienia animacji i przycisk "Wystrzel".
class ControlPanel
{
    SimulationConfig config;
    double angle_deg = 45.0;
    float playbackSpeed = 1.0f;
    bool animate = true;

public:
    static constexpr float Width = 330.0f;

    ControlPanel();

    // Rysuje panel i zwraca akcję wybraną przez użytkownika w tej klatce
    PanelAction draw(const StatusMessage& status, const TrajectoryStats& stats);

    // Config z kątem przeliczonym ze stopni na radiany
    SimulationConfig getConfig() const;
    float getPlaybackSpeed() const { return playbackSpeed; }
    bool isAnimated() const { return animate; }
};
