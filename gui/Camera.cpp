#include "Camera.hpp"

#include <algorithm>
#include <cmath>

namespace gui
{

void Camera::update(const Game& game, double vw, double vh, double dt)
{
    if (!autoFit || vw < 10.0 || vh < 10.0) return;

    const Vec2 muzzle = game.config.muzzle;
    const Target& t = game.target();

    double minX = muzzle.x;
    double maxX = muzzle.x;
    double maxY = muzzle.y;

    maxX = std::max(maxX, t.center.x + t.radius);
    maxY = std::max(maxY, t.center.y + t.radius);

    const bool showFlight = !game.flight().empty() &&
                            (game.phase() == Phase::Flying || game.sinceFinish() < 2.0);
    if (showFlight)
    {
        maxX = std::max(maxX, muzzle.x + std::max(game.flight().range, 0.0));
        maxY = std::max(maxY, game.flight().maxHeight);
    }
    if (game.showPreview && game.previewValid())
    {
        maxX = std::max(maxX, muzzle.x + std::max(game.preview().range, 0.0));
        maxY = std::max(maxY, game.preview().maxHeight);
    }

    const double bw = std::max(maxX - minX, 1e-3);
    const double bh = std::max(maxY, bw * 0.05);
    const double desiredScale = std::min(vw * 0.82 / bw, vh * 0.68 / bh);
    const double desiredCx = (minX + maxX) * 0.5;
    const double groundFrac = 0.16;
    const double desiredCy = (0.5 - groundFrac) * vh / desiredScale;

    if (!initialized)
    {
        scale = desiredScale;
        cx = desiredCx;
        cy = desiredCy;
        initialized = true;
        return;
    }

    const double k = 1.0 - std::exp(-dt * 6.0);
    scale = std::exp(std::log(scale) + (std::log(desiredScale) - std::log(scale)) * k);
    cx += (desiredCx - cx) * k;
    cy = (0.5 - groundFrac) * vh / scale;
}

void Camera::zoomAt(double screenX, double screenY, double vw, double vh, double factor)
{
    const double wx = worldX(screenX, vw);
    const double wy = worldY(screenY, vh);
    scale = std::clamp(scale * factor, 1e-6, 1e7);
    cx = wx - (screenX - vw * 0.5) / scale;
    cy = wy - (screenY - vh * 0.5) / scale;
}

void Camera::pan(double dScreenX, double dScreenY)
{
    cx -= dScreenX / scale;
    cy -= dScreenY / scale;
}

}
