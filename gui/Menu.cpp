#include "Menu.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numbers>

#include "PhysicsCalculations.hpp"
#include "Presets.hpp"
#include "imgui.h"

namespace gui
{

namespace
{

constexpr double kRadToDeg = 180.0 / std::numbers::pi;
constexpr double kMaxAngleDeg = 89.9;

bool dragD(const char* label, double* v, float speed, double mn, double mx, const char* fmt, bool logarithmic = false)
{
    ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp;
    if (logarithmic) flags |= ImGuiSliderFlags_Logarithmic;
    return ImGui::DragScalar(label, ImGuiDataType_Double, v, speed, &mn, &mx, fmt, flags);
}

bool sliderD(const char* label, double* v, double mn, double mx, const char* fmt)
{
    return ImGui::SliderScalar(label, ImGuiDataType_Double, v, &mn, &mx, fmt, ImGuiSliderFlags_AlwaysClamp);
}

void helpMarker(const char* text)
{
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 24.0f);
        ImGui::TextUnformatted(text);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

} // namespace

void drawMenu(Game& game, Camera& camera, const SimulationConfig& defaults, float windowHeight)
{
    SimulationConfig& cfg = game.config;
    bool changed = false;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(kMenuWidth, windowHeight));
    ImGui::Begin("Armata - panel sterowania", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::PushItemWidth(-150.0f);

    const bool flying = game.phase() == Phase::Flying;
    ImGui::Text("Strzaly: %d   Trafienia: %d   Punkty: %d", game.shots(), game.hits(), game.totalPoints());

    ImGui::BeginDisabled(flying);
    if (ImGui::Button("WYSTRZEL  [Spacja]", ImVec2(180, 34))) game.fire();
    ImGui::SameLine();
    if (ImGui::Button("Nowy cel [R]", ImVec2(-1, 34))) game.newTarget();
    ImGui::EndDisabled();
    if (ImGui::Button("Wyzeruj wynik")) game.resetScore();
    ImGui::SameLine();
    if (ImGui::Button("Przywroc startowy config"))
    {
        game.resetConfig(defaults);
    }

    if (!game.configError().empty())
        ImGui::TextColored(ImVec4(1.f, 0.45f, 0.35f, 1.f), "Blad configu: %s", game.configError().c_str());

    const ShotReport& rep = game.report();
    if (rep.valid)
    {
        ImVec4 col(1.f, 0.55f, 0.55f, 1.f);
        if (rep.error) col = ImVec4(1.f, 0.7f, 0.3f, 1.f);
        else if (rep.points > 0) col = ImVec4(0.5f, 1.f, 0.6f, 1.f);
        ImGui::TextColored(col, "%s", rep.text.c_str());
        if (!rep.error)
            ImGui::TextDisabled("Lot: %.2f s, upadek w %s", rep.flightTime, formatDistance(rep.landing.x - cfg.muzzle.x).c_str());
    }
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Armata", ImGuiTreeNodeFlags_DefaultOpen))
    {
        double deg = cfg.angle_rad * kRadToDeg;
        if (sliderD("Kat strzalu", &deg, 0.0, kMaxAngleDeg, "%.1f deg"))
        {
            cfg.angle_rad = deg / kRadToDeg;
            changed = true;
        }
        helpMarker("Strzalki Gora/Dol: +-0.5 deg (Shift: 0.1, Ctrl: 5).\nCtrl+klik w suwak pozwala wpisac wartosc.");

        changed |= dragD("Sila [N]", &cfg.force_N, 50.f, 1.0, 1e8, "%.1f", true);
        helpMarker("Strzalki Lewo/Prawo zmieniaja sile o 2% (Shift: 0.5%).");
        changed |= dragD("Czas impulsu [s]", &cfg.impulseTime_s, 0.0005f, 1e-4, 10.0, "%.4f", true);
        changed |= dragD("Masa pocisku [kg]", &cfg.mass_kg, 0.05f, 1e-3, 1e4, "%.3f", true);
        changed |= dragD("Lufa X [m]", &cfg.muzzle.x, 0.1f, 0.0, 1e6, "%.2f");
        changed |= dragD("Lufa Y [m]", &cfg.muzzle.y, 0.1f, 0.0, 1e6, "%.2f");
        helpMarker("Polozenie lufy wzgledem punktu (0, 0). Y > 0 stawia armate na podescie.");

        if (ImGui::BeginCombo("Preset armaty", "wybierz..."))
        {
            for (const CannonPreset& p : kCannonPresets)
            {
                if (ImGui::Selectable(p.name))
                {
                    cfg.force_N = p.force_N;
                    cfg.impulseTime_s = p.impulseTime_s;
                    cfg.mass_kg = p.mass_kg;
                    changed = true;
                }
            }
            ImGui::EndCombo();
        }
    }

    if (ImGui::CollapsingHeader("Grawitacja", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const float colW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.f) / 3.f;
        int col = 0;
        for (const GravityPreset& p : kGravityPresets)
        {
            const bool active = std::fabs(cfg.gravity - p.value) < 1e-9;
            char label[64];
            std::snprintf(label, sizeof label, "%s\n%.2f", p.name, p.value);
            if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            if (ImGui::Button(label, ImVec2(colW, 38)))
            {
                cfg.gravity = p.value;
                changed = true;
            }
            if (active) ImGui::PopStyleColor();
            if (++col % 3 != 0) ImGui::SameLine();
        }
        if (col % 3 != 0) ImGui::NewLine();
        changed |= dragD("Wlasna g [m/s^2]", &cfg.gravity, 0.05f, 0.001, 5000.0, "%.4f", true);
    }

    if (ImGui::CollapsingHeader("Parametry symulacji"))
    {
        changed |= dragD("Krok czasu [s]", &cfg.timeStep_s, 0.0001f, 1e-4, 0.1, "%.4f", true);
        changed |= dragD("Max czas lotu [s]", &cfg.maxTime_s, 0.5f, 0.1, 600.0, "%.1f");
        const double steps = cfg.maxTime_s / cfg.timeStep_s;
        ImGui::TextDisabled("Maks. liczba krokow: %.0f (limit 1 000 000)", steps);
    }

    if (ImGui::CollapsingHeader("Cel i wizualizacja", ImGuiTreeNodeFlags_DefaultOpen))
    {
        dragD("Rozmiar celu [% zasiegu]", &game.targetSizePercent, 0.05f, 0.5, 20.0, "%.1f %%");
        ImGui::Checkbox("Cele w powietrzu (na slupie)", &game.elevatedTargets);
        ImGui::Checkbox("Automatyczny nowy cel po trafieniu", &game.autoRespawn);
        if (ImGui::Checkbox("Podglad toru (dla testow)", &game.showPreview)) changed = true;
        ImGui::Checkbox("Auto-kamera", &camera.autoFit);
        helpMarker("Kolko myszy = zoom, prawy/srodkowy przycisk + ruch = przesuwanie (wylacza auto-kamere).");
        ImGui::Checkbox("Auto-tempo (lot ok. 4 s)", &game.autoTempo);
        if (!game.autoTempo) dragD("Tempo animacji", &game.timeScale, 0.02f, 0.05, 200.0, "%.2fx", true);
    }

    if (ImGui::CollapsingHeader("Informacje", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const ProjectileState s0 = initialState(cfg);
        ImGui::Text("Impuls:             %.2f N*s", impulse(cfg));
        ImGui::Text("Predkosc poczatkowa: %.2f m/s", initialSpeed(cfg));
        ImGui::Text("  vx = %.2f   vy = %.2f m/s", s0.velocity.x, s0.velocity.y);
        ImGui::Text("Przyspieszenie pion. (physics): %.4f m/s^2", s0.acceleration.y);
        if (std::fabs(s0.acceleration.y + cfg.gravity) > 1e-6 * cfg.gravity)
        {
            ImGui::TextColored(ImVec4(1.f, 0.8f, 0.3f, 1.f), "Uwaga: to nie jest -g (%.4f)!", -cfg.gravity);
            helpMarker("Modul physics liczy force = -g/m, a acceleration = force/m, czyli a = -g/m^2.\nDla m = 1 kg zgadza sie z -g. GUI nie zmienia katalogu physics.");
        }
        ImGui::Text("Maks. zasieg (45 deg): %s", formatDistance(game.reachRange()).c_str());
        const Target& t = game.target();
        ImGui::Text("Cel: dystans %s, wysokosc %s, promien %s",
                    formatDistance(t.center.x - cfg.muzzle.x).c_str(),
                    formatDistance(t.center.y).c_str(),
                    formatDistance(t.radius).c_str());
        if (game.showPreview && game.previewValid())
            ImGui::Text("Podglad: zasieg %s, max wys. %s, czas %.2f s",
                        formatDistance(game.preview().range).c_str(),
                        formatDistance(game.preview().maxHeight).c_str(),
                        game.preview().flightTime);
        ImGui::Spacing();
        ImGui::TextDisabled("Sterowanie: Spacja - strzal, R - nowy cel,\nGora/Dol - kat, Lewo/Prawo - sila.");
        ImGui::TextDisabled("Tarcza: zolty srodek 100, czerwony 50, bialy 25 pkt.");
    }

    if (changed) game.configChanged();

    ImGui::PopItemWidth();
    ImGui::End();
}

} // namespace gui
