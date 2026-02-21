#pragma once

#include "Product.h"
#include <map>
#include <string>
#include <vector>

namespace DataReader
{

std::map<std::string, double> read_minerals(const std::string& filename);
std::vector<Product> read_products(const std::string& filename);

} // namespace DataReader
