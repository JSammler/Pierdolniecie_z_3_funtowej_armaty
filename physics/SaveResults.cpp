#include <array>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctime>
#include <limits>
#include <filesystem>

#include "SaveResults.hpp"

std::string getFileName(std::time_t now)
{
    std::error_code ec;
    std::filesystem::create_directories("../results/csv", ec);
    std::filesystem::create_directories("../results/bin", ec);

    std::tm time{};
#ifdef _WIN32
    localtime_s(&time, &now);
#else
    localtime_r(&now, &time);
#endif
    std::ostringstream ss;
    ss << std::put_time(&time, "%Y_%m_%d-%H_%M_%S");
    return ss.str();
}

SaveResult SaveCsv(StepsStack* start, std::string fileName)
{
    const char sep = ',';
    const std::array<std::string, 7> names = {"time","position_x", "position_y","velocity_x", "velocity_y","acceleration_x", "acceleration_y"};

    std::ofstream file("../results/csv/" + fileName + ".csv");
    if (!file) { return {false, "Cannot open file: " + fileName}; }
    file << std::setprecision(std::numeric_limits<double>::max_digits10);

    for (int i = 0; i < 7; i++) {
        if (i > 0) { file << sep; }
        file << names[i];
    }
    file << '\n';

    for (StepsStack* p = start; p; p = p->next)
    {
        auto a = p->state.getArray();
        for (int i = 0; i < 7; i++)
        {
            if (i > 0) { file << sep; }
            file << a[i];
        }
        file << '\n';
    }

    if (!file) { return {false, "An error occurred while writing file: " + fileName}; }
    return {};
}

SaveResult SaveBin(StepsStack* start, int count, std::string fileName)
{
    std::ofstream file("../results/bin/" + fileName + ".bin", std::ios::binary);
    if (!file) { return {false, "Cannot open file: " + fileName}; }

    file.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (StepsStack* p = start; p; p = p->next)
    {
        auto a = p->state.getArray();
        file.write(reinterpret_cast<const char*>(a.data()), sizeof(a));
    }

    if (!file) { return {false, "An error occurred while writing file: " + fileName}; }
    return {};
}