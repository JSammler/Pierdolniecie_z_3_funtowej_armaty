#pragma once

struct Vec2 {
    double x = 0.0;
    double y = 0.0;

    Vec2 operator*(double k) const { return {x * k, y * k}; }
    Vec2 operator/(double k) const { return {x / k, y / k}; }
    Vec2 operator+(Vec2 b) const { return {x + b.x, y + b.y}; }
    Vec2 operator-(Vec2 b) const { return {x - b.x, y - b.y}; }
};