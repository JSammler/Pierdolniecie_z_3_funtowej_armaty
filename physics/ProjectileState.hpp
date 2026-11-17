#pragma once
#include <array>

#include "Vec2.hpp"

struct ProjectileState
{
    double time = 0.0;
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;

    std::array<double, 7> getArray()
    {
        return {time, position.x, position.y, velocity.x, velocity.y, acceleration.x, acceleration.y};
    }
};