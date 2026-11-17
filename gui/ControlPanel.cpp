#include <numbers>

#include <imgui.h>

#include "ControlPanel.hpp"

namespace
{
    constexpr double RadToDeg = 180.0 / std::numbers::pi;

    void inputDouble(const char* label, double* value, double step, const char* format)
    {
        ImGui::InputDouble(label, value, step, step * 10.0, format);
    }
}

ControlPanel::ControlPanel()
{
    angle_deg = config.angle_rad * RadToDeg;
}

SimulationConfig ControlPanel::getConfig() const
{
    SimulationConfig result = config;
    result.angle_rad = angle_deg / RadToDeg;
    return result;
}

PanelAction ControlPanel::draw(const StatusMessage& status, const TrajectoryStats& stats)
{
    PanelAction action = PanelAction::None;
    const ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos({0.0f, 0.0f});
    ImGui::SetNextWindowSize({Width, io.DisplaySize.y});
    ImGui::Begin("Parametry", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

    ImGui::PushItemWidth(150.0f);

    ImGui::SeparatorText("Armata");
    inputDouble("Siła [N]", &config.force_N, 100.0, "%.1f");
    inputDouble("Czas impulsu [s]", &config.impulseTime_s, 0.001, "%.4f");
    inputDouble("Masa kuli [kg]", &config.mass_kg, 0.1, "%.2f");
    constexpr double angleMin = 0.1, angleMax = 89.9;
    ImGui::SliderScalar("Kąt [°]", ImGuiDataType_Double, &angle_deg, &angleMin, &angleMax, "%.1f");
    inputDouble("Lufa x [m]", &config.muzzle.x, 1.0, "%.2f");
    inputDouble("Lufa y [m]", &config.muzzle.y, 1.0, "%.2f");

    ImGui::SeparatorText("Otoczenie");
    inputDouble("Grawitacja [m/s²]", &config.gravity, 0.1, "%.5f");

    ImGui::SeparatorText("Symulacja");
    inputDouble("Krok czasu [s]", &config.timeStep_s, 0.001, "%.4f");
    inputDouble("Maks. czas lotu [s]", &config.maxTime_s, 1.0, "%.1f");

    ImGui::SeparatorText("Animacja");
    ImGui::Checkbox("Animuj lot", &animate);
    ImGui::BeginDisabled(!animate);
    ImGui::SliderFloat("Prędkość", &playbackSpeed, 0.1f, 50.0f, "%.1fx", ImGuiSliderFlags_Logarithmic);
    ImGui::EndDisabled();

    ImGui::PopItemWidth();

    ImGui::Spacing();
    if (ImGui::Button("Wystrzel", {-FLT_MIN, 40.0f})) action = PanelAction::Fire;
    const float halfWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0f;
    if (ImGui::Button("Wyczyść", {halfWidth, 0.0f})) action = PanelAction::Clear;
    ImGui::SameLine();
    if (ImGui::Button("Domyślne", {halfWidth, 0.0f}))
    {
        config = SimulationConfig();
        angle_deg = config.angle_rad * RadToDeg;
    }

    ImGui::SeparatorText("Wynik");
    if (!status.text.empty())
    {
        if (status.error) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        ImGui::TextWrapped("%s", status.text.c_str());
        if (status.error) ImGui::PopStyleColor();
    }
    if (stats.valid)
    {
        ImGui::Text("Czas lotu:     %.3f s", stats.flightTime);
        ImGui::Text("Zasięg (x):    %.2f m", stats.range);
        ImGui::Text("Maks. wys.:    %.2f m", stats.maxHeight);
        ImGui::Text("Liczba kroków: %d", stats.points);
    }

    ImGui::End();
    return action;
}
