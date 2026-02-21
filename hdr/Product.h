#pragma once

#include <string>
#include <map>

// Data structure for a product
struct Product
{
    std::string name;
    double value;
    std::map<std::string, double> mineral_consumption;
    double production_time; // Time in seconds to produce one unit
};
