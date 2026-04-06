#pragma once

#include <string>
#include <vector>
#include "csv_object.hpp"

struct Fuel : public CSVObject
{
    std::string name;
    double power;
    double duration;

    Fuel() = default;
    Fuel(std::string n, double p, double d) : name(n), power(p), duration(d) {}

    static std::vector<Fuel> readCSV(const std::string& filename);

    // CSVObject implementation
    std::string get_title() const override { return "Fuels"; }
    std::vector<std::string> get_headers() const override;
    std::vector<std::string> get_values() const override;
};
