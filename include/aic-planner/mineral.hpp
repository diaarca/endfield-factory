#pragma once

#include "csv_object.hpp"
#include <map>
#include <string>
#include <vector>

struct Mineral : public CSVObject
{
    std::string name;
    double limit;

    Mineral() = default;
    Mineral(std::string n, double l) : name(n), limit(l) {}

    static std::vector<Mineral> readCSV(const std::string& filename);

    // CSVObject implementation
    std::string get_title() const override { return "Minerals"; }
    std::vector<std::string> get_headers() const override;
    std::vector<std::string> get_values() const override;
};
