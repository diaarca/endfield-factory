#include "fuel.hpp"
#include <iostream>

std::vector<Fuel> Fuel::readCSV(const std::string& filename)
{
    std::vector<Fuel> fuels;
    auto data = CSVObject::read_file(filename);
    if (data.empty())
        return fuels;

    for (size_t i = 1; i < data.size(); ++i)
    {
        if (data[i].size() >= 3)
        {
            Fuel f;
            f.name = data[i][0];
            f.power = std::stod(data[i][1]);
            f.duration = std::stod(data[i][2]);
            fuels.push_back(f);
        }
    }
    return fuels;
}

std::vector<std::string> Fuel::get_headers() const
{
    return {"Fuel", "Power", "Duration"};
}

std::vector<std::string> Fuel::get_values() const
{
    return {name, std::to_string(power), std::to_string(duration)};
}
