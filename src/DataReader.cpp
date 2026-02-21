#include "DataReader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace
{
// Helper function to parse a single CSV line
std::vector<std::string> parse_csv_line(const std::string& line)
{
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, ','))
    {
        item.erase(item.find_last_not_of("") + 1);
        result.push_back(item);
    }
    return result;
}
} // namespace

namespace DataReader
{

std::map<std::string, double> read_minerals(const std::string& filename)
{
    std::map<std::string, double> minerals;
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open minerals file: " + filename);
    }
    std::cout << "DEBUG: Reading minerals from: " << filename << std::endl;
    std::string line;
    std::getline(file, line); // Skip header

    while (std::getline(file, line))
    {
        auto row = parse_csv_line(line);
        if (row.size() >= 2)
        {
            minerals[row[0]] = std::stod(row[1]);
            std::cout << "DEBUG:   Read Mineral: " << row[0]
                      << ", Limit: " << row[1] << std::endl;
        }
    }
    return minerals;
}

std::vector<Product> read_products(const std::string& filename)
{
    std::vector<Product> products;
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open products file: " + filename);
    }
    std::cout << "DEBUG: Reading products from: " << filename << std::endl;
    std::string line;

    std::getline(file, line);
    std::vector<std::string> header = parse_csv_line(line);
    std::cout << "DEBUG:   Header: ";
    for (const auto& h : header)
        std::cout << h << " | ";
    std::cout << std::endl;

    int name_idx = -1, value_idx = -1, time_idx = -1;
    std::vector<std::pair<std::string, int>> mineral_indices;

    for (int i = 0; i < header.size(); ++i)
    {
        if (header[i] == "product" || header[i] == "Item_Name")
            name_idx = i;
        else if (header[i] == "value" || header[i] == "Trading_Value")
            value_idx = i;
        else if (header[i] == "time" || header[i] == "Production_Time")
            time_idx = i;
        else
        {
            mineral_indices.push_back({header[i], i});
        }
    }

    if (name_idx == -1 || value_idx == -1 || time_idx == -1)
    {
        throw std::runtime_error(
            "Products CSV header is missing required columns "
            "(product/Item_Name, value/Trading_Value, time/Production_Time).");
    }

    while (std::getline(file, line))
    {
        auto row = parse_csv_line(line);
        if (row.size() != header.size())
            continue;

        Product p;
        p.name = row[name_idx];
        p.value = std::stod(row[value_idx]);
        p.production_time = std::stod(row[time_idx]);

        std::cout << "DEBUG:   Read Product: " << p.name
                  << ", Value: " << p.value << ", Time: " << p.production_time;

        for (const auto& mineral_info : mineral_indices)
        {
            p.mineral_consumption[mineral_info.first] =
                std::stod(row[mineral_info.second]);
            std::cout << ", " << mineral_info.first << ": "
                      << p.mineral_consumption[mineral_info.first];
        }
        std::cout << std::endl;
        products.push_back(p);
    }
    return products;
}

} // namespace DataReader
