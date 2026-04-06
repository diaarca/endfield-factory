#include "facility.hpp"
#include <iostream>

std::map<std::string, double> Facility::readCSV(const std::string& filename)
{
    std::map<std::string, double> facilities;
    auto data = CSVObject::read_file(filename);
    if (data.empty())
        return facilities;

    for (size_t i = 1; i < data.size(); ++i)
    {
        if (data[i].size() >= 2)
        {
            facilities[data[i][0]] = std::stod(data[i][1]);
        }
    }
    return facilities;
}

std::vector<std::string> Facility::get_headers() const
{
    return {"Facility", "Power"};
}

std::vector<std::string> Facility::get_values() const
{
    return {name, std::to_string(power)};
}
