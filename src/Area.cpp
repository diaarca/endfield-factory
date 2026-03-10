#include "Area.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>

std::vector<Area> Area::readCSV(const std::string& filename)
{
    std::vector<Area> areas;
    std::ifstream file(filename);
    if (!file.is_open())
    {
        return areas;
    }
    std::string line;
    if (!std::getline(file, line))
        return areas;

    std::vector<std::string> header = CSVReader::parse_line(line);
    int name_idx = -1, width_idx = -1, height_idx = -1, depot_w_idx = -1,
        depot_h_idx = -1;
    std::vector<std::pair<std::string, int>> facility_indices;

    for (int i = 0; i < (int)header.size(); ++i)
    {
        if (header[i] == "area" || header[i] == "Area_Name")
            name_idx = i;
        else if (header[i] == "pac_width" || header[i] == "Width")
            width_idx = i;
        else if (header[i] == "pac_height" || header[i] == "Height")
            height_idx = i;
        else if (header[i] == "pac_depot" || header[i] == "pac_depot_width" ||
                 header[i] == "Depot_W")
            depot_w_idx = i;
        else if (header[i] == "pac_depot_height" || header[i] == "Depot_H")
            depot_h_idx = i;
        else
        {
            facility_indices.push_back({header[i], i});
        }
    }

    if (name_idx == -1)
        return areas;

    while (std::getline(file, line))
    {
        auto row = CSVReader::parse_line(line);
        if (row.size() != header.size())
            continue;

        Area a;
        a.name = row[name_idx];
        a.pac_width = (width_idx != -1) ? std::stod(row[width_idx]) : 0;
        a.pac_height = (height_idx != -1) ? std::stod(row[height_idx]) : 0;
        a.pac_depot_width = (depot_w_idx != -1) ? std::stod(row[depot_w_idx]) : 0;
        a.pac_depot_height =
            (depot_h_idx != -1) ? std::stod(row[depot_h_idx]) : 0;

        for (const auto& f_info : facility_indices)
        {
            a.area_facilities[f_info.first] = std::stod(row[f_info.second]);
        }
        areas.push_back(a);
    }
    return areas;
}

void Area::print_table(const std::vector<Area>& areas)
{
    if (areas.empty())
        return;

    // Identify all facilities across all areas
    std::map<std::string, bool> all_facilities;
    for (const auto& a : areas)
    {
        for (const auto& f : a.area_facilities)
        {
            if (f.second > 0)
                all_facilities[f.first] = true;
        }
    }

    std::cout << "\n--- Areas ---\n";
    std::cout << std::left << std::setw(15) << "Area" << " | ";
    for (const auto& f : all_facilities)
    {
        std::string fname = f.first;
        if (fname.length() > 5)
            fname = fname.substr(0, 5);
        std::cout << std::setw(6) << fname << " | ";
    }
    std::cout << std::setw(3) << "DW" << " | " << std::setw(3) << "DH" << " | "
              << std::setw(3) << "W" << " | " << std::setw(3) << "H" << "\n";
    std::cout << std::string(80, '-') << "\n";

    for (const auto& a : areas)
    {
        std::cout << std::left << std::setw(15)
                  << (a.name.length() > 15 ? a.name.substr(0, 15) : a.name)
                  << " | ";
        for (const auto& f : all_facilities)
        {
            double val = 0;
            if (a.area_facilities.count(f.first))
                val = a.area_facilities.at(f.first);
            std::cout << std::setw(6) << val << " | ";
        }
        std::cout << std::setw(3) << a.pac_depot_width << " | " << std::setw(3)
                  << a.pac_depot_height << " | " << std::setw(3) << a.pac_width
                  << " | " << std::setw(3) << a.pac_height << "\n";
    }
}
