#include <exception>
#include <memory>

#include <imgui.h>

#include "physics/Simulation.hpp"
#include "physics/SimulationConfig.hpp"
#include "physics/Validation.hpp"

#include "gui/Window.hpp"
#include "gui/ControlPanel.hpp"
#include "gui/TrajectoryView.hpp"

// Łączy logikę (physics) z interfejsem (gui)
int main()
{
    Window window(1280, 720, "Pierdolniecie z 3 funtowej armaty");
    ControlPanel panel;
    TrajectoryView view;

    // Simulation jest właścicielem węzłów listy rysowanej przez TrajectoryView
    std::unique_ptr<Simulation> simulation;
    StatusMessage status{"Ustaw parametry i kliknij \"Wystrzel\"."};

    while (!window.shouldClose())
    {
        window.beginFrame();

        switch (panel.draw(status, view.getStats()))
        {
        case PanelAction::Fire:
        {
            SimulationConfig config = panel.getConfig();

            if (ValidationResult validation = Validation_Config(config); !validation)
            {
                status = {validation.errorMessage, true};
                break;
            }

            try
            {
                auto newSimulation = std::make_unique<Simulation>(config);
                ValidationResult result = newSimulation->Run();
                view.setTrajectory(newSimulation->getResult());
                simulation = std::move(newSimulation);
                status = {result.errorMessage, !result};
            }
            catch (const std::exception& e)
            {
                status = {e.what(), true};
            }
            break;
        }
        case PanelAction::Clear:
            view.clear();
            simulation.reset();
            status = {};
            break;
        case PanelAction::None:
            break;
        }

        const ImVec2 size = ImGui::GetIO().DisplaySize;
        view.draw(ControlPanel::Width, 0.0f, size.x - ControlPanel::Width, size.y,
                  panel.isAnimated(), panel.getPlaybackSpeed());

        window.endFrame();
    }
}
