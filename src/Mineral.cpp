#include "mineral.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>

std::vector<Mineral> Mineral::readCSV(const std::string& filename)
{
    std::vector<Mineral> minerals;
    auto data = CSVObject::read_file(filename);
    if (data.empty())
        return minerals;

    for (size_t i = 1; i < data.size(); ++i)
    {
        if (data[i].size() >= 2)
        {
            minerals.push_back({data[i][0], std::stod(data[i][1])});
        }
    }
    return minerals;
}

std::vector<std::string> Mineral::get_headers() const
{
    return {"Mineral", "Limit"};
}

std::vector<std::string> Mineral::get_values() const
{
    return {name, std::to_string(limit)};
}
