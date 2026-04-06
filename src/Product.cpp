#include "product.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>

std::vector<Product>
Product::readCSV(const std::string& filename,
                 const std::vector<Mineral>& mineral_limits)
{
    std::vector<Product> products;
    auto data = CSVObject::read_file(filename);
    if (data.empty())
        return products;

    std::vector<std::string> header = data[0];

    int name_idx = -1, value_idx = -1, time_idx = -1, depot_idx = -1,
        width_idx = -1, height_idx = -1;
    std::vector<std::pair<std::string, int>> mineral_indices;
    std::vector<std::pair<std::string, int>> facility_indices;

    for (int i = 0; i < (int)header.size(); ++i)
    {
        if (header[i] == "product" || header[i] == "Item_Name")
            name_idx = i;
        else if (header[i] == "value" || header[i] == "Trading_Value")
            value_idx = i;
        else if (header[i] == "time" || header[i] == "Production_Time")
            time_idx = i;
        else if (header[i] == "depot")
            depot_idx = i;
        else if (header[i] == "width")
            width_idx = i;
        else if (header[i] == "height")
            height_idx = i;
        else if (std::any_of(mineral_limits.begin(), mineral_limits.end(),
                             [&](const Mineral& m)
                             { return m.name == header[i]; }))
        {
            mineral_indices.push_back({header[i], i});
        }
        else
        {
            facility_indices.push_back({header[i], i});
        }
    }

    if (name_idx == -1 || value_idx == -1 || time_idx == -1)
    {
        return products;
    }

    for (size_t i = 1; i < data.size(); ++i)
    {
        auto row = data[i];
        if (row.size() != header.size())
            continue;

        Product p;
        p.name = row[name_idx];
        p.value = std::stod(row[value_idx]);
        p.production_time = std::stod(row[time_idx]);
        p.factory_depot = (depot_idx != -1) ? std::stod(row[depot_idx]) : 0;
        p.factory_width = (width_idx != -1) ? std::stod(row[width_idx]) : 0;
        p.factory_height = (height_idx != -1) ? std::stod(row[height_idx]) : 0;

        for (const auto& m_info : mineral_indices)
        {
            p.mineral_consumption[m_info.first] = std::stod(row[m_info.second]);
        }
        for (const auto& f_info : facility_indices)
        {
            p.factory_facilities[f_info.first] = std::stod(row[f_info.second]);
        }
        products.push_back(p);
    }
    return products;
}

std::vector<std::string> Product::get_headers() const
{
    std::vector<std::string> h = {"Product", "Val", "T", "D", "W", "H"};
    for (auto const& [name, val] : mineral_consumption)
    {
        h.push_back(name);
    }
    for (auto const& [name, val] : factory_facilities)
    {
        h.push_back(name);
    }
    return h;
}

std::vector<std::string> Product::get_values() const
{
    std::vector<std::string> v = {name,
                                  std::to_string(value),
                                  std::to_string(production_time),
                                  std::to_string(factory_depot),
                                  std::to_string(factory_width),
                                  std::to_string(factory_height)};
    for (auto const& [name, val] : mineral_consumption)
    {
        v.push_back(std::to_string(val));
    }
    for (auto const& [name, val] : factory_facilities)
    {
        v.push_back(std::to_string(val));
    }
    return v;
}
