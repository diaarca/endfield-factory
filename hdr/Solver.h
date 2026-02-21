#pragma once

#include "Product.h"
#include <map>
#include <string>
#include <vector>

class Solver
{
  public:
    void solve(const std::vector<Product>& products,
               const std::map<std::string, double>& mineral_limits);
};
