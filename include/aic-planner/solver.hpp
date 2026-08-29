#pragma once

#include "area.hpp"
#include "fuel.hpp"
#include "mineral.hpp"
#include "ortools/sat/cp_model.h"
#include "product.hpp"
#include "region.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

#define NB_HOUR_BEFORE_OVERFLOW 48

class Solver
{
  public:
    Solver(const std::vector<Product>& products,
           const std::vector<Mineral>& mineral_limits,
           const std::vector<Area>& areas,
           const std::vector<Fuel>& fuels,
           const std::map<std::string, double>& facility_power,
           const Region& region);
    void solve();

  private:
    void instantiateVariables();
    void declareConstraints();
    bool solveModel();
    void displaySolution();

    const std::vector<Product>& _products;
    const std::vector<Mineral>& _mineral_limits;
    const std::vector<Area>& _areas;
    const std::vector<Fuel>& _fuels;
    const std::map<std::string, double>& _facility_power;
    const Region& _region;

    operations_research::sat::CpModelBuilder _cp_model;
    operations_research::sat::CpSolverResponse _response;
    std::vector<operations_research::sat::IntVar> _qty_produced;
    std::vector<std::vector<operations_research::sat::IntVar>>
        _factories_in_area;
    std::vector<operations_research::sat::IntVar> _num_batteries_active;
    int64_t _obj_scale_factor;
};
