#pragma once

#include "Camera.hpp"
#include "Game.hpp"
#include "imgui.h"

namespace gui
{

class Renderer
{
public:

    void drawScene(const Game& game, const Camera& cam, double vw, double vh) const;


    void drawLabels(ImDrawList* dl, const Game& game, const Camera& cam, ImVec2 origin, double vw, double vh) const;
};

}
