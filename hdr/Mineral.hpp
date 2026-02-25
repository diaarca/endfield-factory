#pragma once

#include <string>
#include <map>
#include <vector>
#include "CSVReader.hpp"

struct Mineral
{
    std::string name;
    double limit;

    static std::vector<Mineral> readCSV(const std::string& filename);
    static void print_table(const std::vector<Mineral>& minerals);
};
