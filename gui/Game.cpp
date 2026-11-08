#include "Game.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <exception>
#include <numbers>

#include "Simulation.hpp"
#include "Validation.hpp"

namespace gui
{

namespace
{

constexpr double kMaxSteps = 1'000'000.0;
constexpr double kDebounce = 0.15;
constexpr double kAutoFlightDuration = 4.0;
constexpr std::size_t kMaxGhosts = 6;

double distancePointSegment(Vec2 p, Vec2 a, Vec2 b)
{
    const double abx = b.x - a.x;
    const double aby = b.y - a.y;
    const double len2 = abx * abx + aby * aby;
    double t = len2 > 0.0 ? ((p.x - a.x) * abx + (p.y - a.y) * aby) / len2 : 0.0;
    t = std::clamp(t, 0.0, 1.0);
    return std::hypot(p.x - (a.x + abx * t), p.y - (a.y + aby * t));
}

double closestApproach(const std::vector<ProjectileState>& states, Vec2 c)
{
    double best = std::hypot(states.front().position.x - c.x, states.front().position.y - c.y);
    for (std::size_t i = 1; i < states.size(); ++i)
        best = std::min(best, distancePointSegment(c, states[i - 1].position, states[i].position));
    return best;
}

} // namespace

std::string formatDistance(double meters)
{
    char buf[48];
    const double a = std::fabs(meters);
    if (a >= 1000.0) std::snprintf(buf, sizeof buf, "%.2f km", meters / 1000.0);
    else if (a >= 10.0) std::snprintf(buf, sizeof buf, "%.0f m", meters);
    else if (a >= 0.1) std::snprintf(buf, sizeof buf, "%.2f m", meters);
    else std::snprintf(buf, sizeof buf, "%.3f m", meters);
    return buf;
}

bool simulateTrajectory(const SimulationConfig& config, Trajectory& out, std::string& error)
{
    out = Trajectory{};

    SimulationConfig c = config; // Validation_Config przyjmuje referencję niestałą
    if (const ValidationResult v = Validation_Config(c); !v)
    {
        error = v.errorMessage;
        return false;
    }
    if (c.maxTime_s <= 0.0)
    {
        error = "Max time must be greater than zero";
        return false;
    }
    if (c.maxTime_s / c.timeStep_s > kMaxSteps)
    {
        error = "Too many steps (maxTime / timeStep > 1 000 000)";
        return false;
    }

    try
    {
        Simulation sim(c);
        const ValidationResult run = sim.Run();
        StepsStack head = sim.getResult();

        out.states.reserve(static_cast<std::size_t>(sim.getCount()));
        for (const StepsStack* n = &head; n != nullptr; n = n->next)
            out.states.push_back(n->state);

        // Simulation nie ma destruktora - zwalniamy węzły listy po stronie GUI.
        for (StepsStack* n = head.next; n != nullptr;)
        {
            StepsStack* next = n->next;
            delete n;
            n = next;
        }

        out.landed = run.statusOk; // Run(): {true, "..."} = uderzenie w ziemię, {false, ...} = timeout
        out.endMessage = run.errorMessage;
    }
    catch (const std::exception& e)
    {
        error = e.what();
        out = Trajectory{};
        return false;
    }

    out.timeStep = c.timeStep_s;
    for (const ProjectileState& s : out.states) out.maxHeight = std::max(out.maxHeight, s.position.y);

    const ProjectileState& last = out.states.back();
    out.flightTime = last.time;
    out.landing = last.position;
    if (out.landed && out.states.size() >= 2)
    {
        const ProjectileState& prev = out.states[out.states.size() - 2];
        const double denom = prev.position.y - last.position.y;
        if (denom > 1e-12)
        {
            const double t = std::clamp(prev.position.y / denom, 0.0, 1.0);
            out.landing = {prev.position.x + t * (last.position.x - prev.position.x), 0.0};
        }
    }
    out.range = out.landing.x - c.muzzle.x;
    return true;
}



Game::Game(const SimulationConfig& initial)
    : config(initial), rng_(std::random_device{}())
{
    recomputeReference();
    newTarget();
}

void Game::resetConfig(const SimulationConfig& defaults)
{
    config = defaults;
    configChanged();
}

void Game::resetScore()
{
    shots_ = hits_ = totalPoints_ = 0;
    report_ = ShotReport{};
    ghosts_.clear();
}

void Game::recomputeReference()
{
    dirty_ = false;
    dirtyTimer_ = 0.0;
    configError_.clear();
    previewValid_ = false;
    refValid_ = false;

    SimulationConfig ref = config;
    ref.angle_rad = std::numbers::pi / 4.0;
    Trajectory t;
    std::string err;
    if (simulateTrajectory(ref, t, err) && !t.empty())
    {
        reachRange_ = std::max(t.range, 1e-3);
        reachHeight_ = std::max(t.maxHeight, 1e-3);
        refValid_ = true;
    }
    else
    {
        configError_ = err.empty() ? "Simulation too short - check max time" : err;
    }

    if (showPreview && refValid_)
    {
        std::string perr;
        if (simulateTrajectory(config, preview_, perr) && !preview_.empty()) previewValid_ = true;
        else
        {
            preview_ = Trajectory{};
            if (configError_.empty()) configError_ = perr;
        }
    }
    else
    {
        preview_ = Trajectory{};
    }
}

void Game::newTarget()
{
    if (phase_ == Phase::Flying) return;
    if (dirty_) recomputeReference();

    const double reach = (refValid_ && reachRange_ > 1e-6) ? reachRange_ : 100.0;
    std::uniform_real_distribution<double> distFrac(0.25, 0.85);
    const double d = reach * distFrac(rng_);
    const double r = std::max(reach * targetSizePercent / 100.0, 1e-3);

    double y = r;
    if (elevatedTargets)
    {

        const double h = std::max(d * (1.0 - d / reach) * 0.7, r);
        std::uniform_real_distribution<double> hy(r, h);
        y = hy(rng_);
    }

    target_.center = {config.muzzle.x + d, y};
    target_.radius = r;
    targetHit_ = false;
    respawnIn_ = 0.0;
    report_ = ShotReport{};
    ghosts_.clear();
}

bool Game::fire()
{
    if (phase_ == Phase::Flying) return false;

    Trajectory t;
    std::string err;
    if (!simulateTrajectory(config, t, err) || t.empty())
    {
        report_ = ShotReport{};
        report_.valid = true;
        report_.error = true;
        report_.text = "Strzal niemozliwy: " + (err.empty() ? std::string("symulacja zbyt krotka (max time?)") : err);
        sinceFinish_ = 0.0;
        return false;
    }

    flight_ = std::move(t);
    phase_ = Phase::Flying;
    playT_ = 0.0;
    sinceFire_ = 0.0;
    respawnIn_ = 0.0;
    report_ = ShotReport{};
    ++shots_;
    return true;
}

double Game::animationScale() const
{
    if (autoTempo) return std::max(flight_.flightTime / kAutoFlightDuration, 1.0);
    return std::max(timeScale, 1e-3);
}

void Game::update(double dt)
{
    sinceFire_ += dt;
    sinceFinish_ += dt;

    if (dirty_)
    {
        dirtyTimer_ += dt;
        if (dirtyTimer_ >= kDebounce) recomputeReference();
    }

    if (phase_ == Phase::Flying)
    {
        playT_ += dt * animationScale();
        if (playT_ >= flight_.states.back().time) finishShot();
    }
    else if (respawnIn_ > 0.0)
    {
        respawnIn_ -= dt;
        if (respawnIn_ <= 0.0) newTarget();
    }
}

void Game::finishShot()
{
    phase_ = Phase::Idle;
    sinceFinish_ = 0.0;

    ShotReport r;
    r.valid = true;
    r.landed = flight_.landed;
    r.landing = flight_.landing;
    r.flightTime = flight_.flightTime;
    r.closest = closestApproach(flight_.states, target_.center);

    const double R = target_.radius;
    if (r.closest <= 0.2 * R)      { r.zone = Zone::Bullseye; r.points = 100; r.text = "W dziesiatke! +100 pkt"; }
    else if (r.closest <= 0.5 * R) { r.zone = Zone::Middle;   r.points = 50;  r.text = "Dobry strzal! +50 pkt"; }
    else if (r.closest <= R)       { r.zone = Zone::Outer;    r.points = 25;  r.text = "Trafiony brzeg tarczy +25 pkt"; }
    else
    {
        r.zone = Zone::Miss;
        r.text = "Pudlo - zabraklo " + formatDistance(r.closest - R) + " do tarczy";
    }
    if (!r.landed) r.text += " (lot przerwany po max. czasie)";

    totalPoints_ += r.points;
    if (r.points > 0)
    {
        ++hits_;
        targetHit_ = true;
        if (autoRespawn) respawnIn_ = 1.6;
    }

    std::vector<Vec2> ghost;
    const std::size_t n = flight_.states.size();
    const std::size_t stride = std::max<std::size_t>(1, n / 300);
    for (std::size_t i = 0; i < n; i += stride) ghost.push_back(flight_.states[i].position);
    ghost.push_back(flight_.landing);
    ghosts_.push_back(std::move(ghost));
    while (ghosts_.size() > kMaxGhosts) ghosts_.pop_front();

    report_ = std::move(r);
}

std::size_t Game::ballIndex() const
{
    if (flight_.states.empty()) return 0;
    const double idx = std::clamp(playT_ / flight_.timeStep, 0.0, static_cast<double>(flight_.states.size() - 1));
    return static_cast<std::size_t>(idx);
}

Vec2 Game::ballPosition() const
{
    if (flight_.states.empty()) return config.muzzle;
    const std::size_t i = ballIndex();
    if (i + 1 >= flight_.states.size()) return flight_.landed ? flight_.landing : flight_.states.back().position;

    const double frac = std::clamp(playT_ / flight_.timeStep - static_cast<double>(i), 0.0, 1.0);
    const Vec2 a = flight_.states[i].position;
    const Vec2 b = flight_.states[i + 1].position;
    return {a.x + (b.x - a.x) * frac, a.y + (b.y - a.y) * frac};
}

}
