#pragma once
#include <string>
#include <ctime>

#include "Simulation.hpp"

struct SaveResult
{
    bool statusOk = true;
    std::string errorMessage;

    explicit operator bool() const { return statusOk; }
};

std::string getFileName(std::time_t now = std::time(nullptr));

SaveResult SaveCsv(StepsStack* start, std::string fileName);
SaveResult SaveBin(StepsStack* start, int count, std::string fileName);