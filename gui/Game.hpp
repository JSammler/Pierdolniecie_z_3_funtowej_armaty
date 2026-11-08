#pragma once

#include <cstddef>
#include <deque>
#include <random>
#include <string>
#include <vector>

#include "ProjectileState.hpp"
#include "SimulationConfig.hpp"
#include "Vec2.hpp"


namespace gui
{

struct Trajectory
{
    std::vector<ProjectileState> states;
    double timeStep = 0.001;
    bool landed = false;
    std::string endMessage;
    Vec2 landing{};             // punkt uderzenia (interpolowany do y = 0)
    double flightTime = 0.0;
    double maxHeight = 0.0;
    double range = 0.0;         // odległość w poziomie od lufy do punktu uderzenia
    bool empty() const { return states.size() < 2; }
};

struct Target
{
    Vec2 center{};
    double radius = 1.0;
};

enum class Zone { Miss, Outer, Middle, Bullseye };
enum class Phase { Idle, Flying };

struct ShotReport
{
    bool valid = false;   // czy jest co pokazać
    bool error = false;   // true: strzał się nie odbył (zła konfiguracja)
    Zone zone = Zone::Miss;
    int points = 0;
    double closest = 0.0; // najmniejsza odległość toru od środka tarczy
    double flightTime = 0.0;
    bool landed = true;
    Vec2 landing{};
    std::string text;
};

// Uruchamia Simulation dla podanego configu. Zwraca false i wypełnia error przy błędzie.
bool simulateTrajectory(const SimulationConfig& config, Trajectory& out, std::string& error);

std::string formatDistance(double meters);

class Game
{
public:
    explicit Game(const SimulationConfig& initial);

    // ---- Edytowane z menu ----
    SimulationConfig config;
    double targetSizePercent = 4.0;  // promień tarczy jako % zasięgu
    bool elevatedTargets = false;    // tarcza na słupie w powietrzu
    bool autoRespawn = true;         // nowy cel po trafieniu
    bool autoTempo = true;           // lot trwa ok. 4 s niezależnie od dystansu
    double timeScale = 1.0;          // ręczna skala czasu animacji
    bool showPreview = false;        // pokaż tor dla bieżących ustawień

    // ---- Akcje ----
    void update(double dtReal);
    bool fire();
    void newTarget();
    void resetScore();
    void resetConfig(const SimulationConfig& defaults);
    void configChanged() { dirty_ = true; dirtyTimer_ = 0.0; }

    // ---- Stan (tylko odczyt) ----
    Phase phase() const { return phase_; }
    const Target& target() const { return target_; }
    bool targetHit() const { return targetHit_; }
    const Trajectory& flight() const { return flight_; }
    const Trajectory& preview() const { return preview_; }
    const std::deque<std::vector<Vec2>>& ghosts() const { return ghosts_; }
    const ShotReport& report() const { return report_; }
    const std::string& configError() const { return configError_; }
    double reachRange() const { return reachRange_; }   // zasięg przy 45 stopniach
    double reachHeight() const { return reachHeight_; }
    bool previewValid() const { return previewValid_; }
    double sinceFire() const { return sinceFire_; }
    double sinceFinish() const { return sinceFinish_; }
    int shots() const { return shots_; }
    int hits() const { return hits_; }
    int totalPoints() const { return totalPoints_; }

    std::size_t ballIndex() const;  // indeks stanu, do którego doleciała kula
    Vec2 ballPosition() const;

private:
    void recomputeReference();
    void finishShot();
    double animationScale() const;

    Phase phase_ = Phase::Idle;
    Target target_{};
    bool targetHit_ = false;
    Trajectory flight_;
    Trajectory preview_;
    std::deque<std::vector<Vec2>> ghosts_;
    ShotReport report_;
    std::string configError_;

    double reachRange_ = 100.0;
    double reachHeight_ = 25.0;
    bool refValid_ = false;
    bool previewValid_ = false;

    double playT_ = 0.0;
    double sinceFire_ = 1e9;
    double sinceFinish_ = 1e9;
    double respawnIn_ = 0.0;

    bool dirty_ = false;
    double dirtyTimer_ = 0.0;

    int shots_ = 0;
    int hits_ = 0;
    int totalPoints_ = 0;

    std::mt19937 rng_;
};

} // namespace gui
