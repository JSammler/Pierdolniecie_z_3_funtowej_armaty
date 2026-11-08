#pragma once

#include "../physics/Simulation.hpp"

struct TrajectoryStats
{
    bool valid = false;
    double flightTime = 0.0;
    double range = 0.0;
    double maxHeight = 0.0;
    int points = 0;
};

// Prawa część okna: siatka XY i lot pocisku rysowany w OpenGL
// na podstawie listy jednokierunkowej StepsStack z modułu physics.
class TrajectoryView
{
    // Kopia pierwszego węzła listy; kolejne węzły należą do obiektu Simulation,
    // który musi żyć tak długo, jak długo trajektoria jest wyświetlana.
    StepsStack head{};
    bool hasTrajectory = false;
    TrajectoryStats stats;

    // Granice świata [m] obejmujące całą trajektorię
    double minX = 0.0, maxX = 100.0, maxY = 50.0;

    double playTime = 0.0; // aktualny czas animacji [s]

public:
    void setTrajectory(const StepsStack& head_);
    void clear();

    const TrajectoryStats& getStats() const { return stats; }

    // Rysuje widok w prostokącie (x, y, w, h) w pikselach okna
    void draw(float x, float y, float w, float h, bool animate, float playbackSpeed);
};
