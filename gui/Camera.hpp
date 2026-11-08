#pragma once

#include "Game.hpp"


namespace gui
{

struct Camera
{
    double cx = 0.0;
    double cy = 0.0;
    double scale = 1.0;
    bool autoFit = true;
    bool initialized = false;

    double sx(double wx, double vw) const { return vw * 0.5 + (wx - cx) * scale; }
    double sy(double wy, double vh) const { return vh * 0.5 + (wy - cy) * scale; }
    double worldX(double screenX, double vw) const { return cx + (screenX - vw * 0.5) / scale; }
    double worldY(double screenY, double vh) const { return cy + (screenY - vh * 0.5) / scale; }

    void update(const Game& game, double vw, double vh, double dt);
    void zoomAt(double screenX, double screenY, double vw, double vh, double factor);
    void pan(double dScreenX, double dScreenY);
};

}
