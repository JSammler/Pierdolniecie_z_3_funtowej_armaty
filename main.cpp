#include <exception>
#include <iostream>
#include <numbers>

#include "gui/App.hpp"
#include "physics/SimulationConfig.hpp"

int main()
{
    SimulationConfig config;

    config.angle_rad = 30 * std::numbers::pi / 180;

    try
    {
        gui::App app(config);
        return app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Blad: " << e.what() << '\n';
        return 1;
    }

    // Dostępne po wykonaniu symulacji
    // getResult zwraca StepsStack ze wszystkimi stanami kuli w locie
    // getCount zwraca ilość elementów na stacku
    // getCount / 1000 = czas lotu w sekundach
    // simulation.getResult();
    // simulation.getCount();
}
