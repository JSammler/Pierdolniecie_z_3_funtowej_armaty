#pragma once

#include "Camera.hpp"
#include "Game.hpp"
#include "Renderer.hpp"
#include "SimulationConfig.hpp"

struct GLFWwindow;

namespace gui
{

class App
{
public:
    explicit App(const SimulationConfig& config);

    int run();

private:
    void handleInput(double vw, double vh, double windowHeight);

    SimulationConfig startConfig_;
    Game game_;
    Camera camera_;
    Renderer renderer_;
};

}