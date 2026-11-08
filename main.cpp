#include <numbers>

#include "physics/Simulation.hpp"
#include "physics/SimulationConfig.hpp"

int main()
{
    SimulationConfig config;

    config.angle_rad = 30 * std::numbers::pi / 180;

    Simulation simulation(config);

    simulation.Run();

    // Dostępne po wykonaniu symulacji
    // getResult zwraca StepsStack ze wszystkimi stanami kuli w locie
    // getCount zwraca ilość elementów na stacku
    // getCount / 1000 = czas lotu w sekundach
    // simulation.getResult();
    // simulation.getCount();
}