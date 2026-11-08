#pragma once

// Stałe wartości do szybkiego przełączania w menu.
// Tylko GUI - moduł physics nie jest tu modyfikowany.

namespace gui
{

struct GravityPreset
{
    const char* name;
    double value; // m/s^2
};

inline constexpr GravityPreset kGravityPresets[] = {
    {"Pluton",   0.62},
    {"Ksiezyc",  1.62},
    {"Merkury",  3.70},
    {"Mars",     3.71},
    {"Uran",     8.69},
    {"Wenus",    8.87},
    {"Ziemia",   9.80665},
    {"Saturn",   10.44},
    {"Neptun",   11.15},
    {"Jowisz",   24.79},
    {"Slonce",   274.0},
};

struct CannonPreset
{
    const char* name;
    double force_N;
    double impulseTime_s;
    double mass_kg;
};

// v0 = F * t / m
inline constexpr CannonPreset kCannonPresets[] = {
    {"Armata 3-funtowa (~440 m/s)",  60000.0,  0.01, 1.36078},
    {"Armata 12-funtowa (~370 m/s)", 200000.0, 0.01, 5.44311},
    {"Katapulta (~60 m/s)",          120000.0, 0.01, 20.0},
    {"Wiatrowka (~100 m/s)",         50.0,     0.01, 0.005},
};

} // namespace gui
