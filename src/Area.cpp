#include "area.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>

std::vector<Area> Area::readCSV(const std::string& filename)
{
    std::vector<Area> areas;
    auto data = CSVObject::read_file(filename);
    if (data.empty())
        return areas;

    std::vector<std::string> header = data[0];
    int name_idx = -1, width_idx = -1, height_idx = -1, depot_w_idx = -1,
        depot_h_idx = -1;
    std::vector<std::pair<std::string, int>> facility_indices;

    for (int i = 0; i < (int)header.size(); ++i)
    {
        if (header[i] == "area")
        {
            name_idx = i;
        }
        else if (header[i] == "pac_width")
        {
            width_idx = i;
        }
        else if (header[i] == "pac_height")
        {
            height_idx = i;
        }
        else if (header[i] == "pac_depot_width")
        {
            depot_w_idx = i;
        }
        else if (header[i] == "pac_depot_height")
        {
            depot_h_idx = i;
        }
        else
        {
            facility_indices.push_back({header[i], i});
        }
    }

    if (name_idx == -1 || width_idx == -1 || height_idx == -1 ||
        depot_w_idx == -1 || depot_h_idx == -1)
    {
        return areas;
    }

    for (size_t i = 1; i < data.size(); ++i)
    {
        auto row = data[i];
        if (row.size() != header.size())
        {
            continue;
        }

        Area a;
        a.name = row[name_idx];
        a.pac_width = std::stod(row[width_idx]);
        a.pac_height = std::stod(row[height_idx]);
        a.pac_depot_width = std::stod(row[depot_w_idx]);
        a.pac_depot_height = std::stod(row[depot_h_idx]);

        for (const auto& f_info : facility_indices)
        {
            a.area_facilities[f_info.first] = std::stod(row[f_info.second]);
        }
        areas.push_back(a);
    }
    return areas;
}

std::vector<std::string> Area::get_headers() const
{
    std::vector<std::string> h = {"Area", "W", "H", "DW", "DH"};
    for (auto const& [name, val] : area_facilities)
    {
        h.push_back(name);
    }
    return h;
}

std::vector<std::string> Area::get_values() const
{
    std::vector<std::string> v = {
        name, std::to_string(pac_width), std::to_string(pac_height),
        std::to_string(pac_depot_width), std::to_string(pac_depot_height)};
    for (auto const& [name, val] : area_facilities)
    {
        v.push_back(std::to_string(val));
    }
    return v;
}
