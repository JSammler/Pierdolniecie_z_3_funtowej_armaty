#pragma once

#include "Vec2.hpp"

struct ProjectileState
{
    double time = 0.0;
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
};