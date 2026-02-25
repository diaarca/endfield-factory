#pragma once

#include "Area.hpp"
#include "Mineral.hpp"
#include "Product.hpp"
#include <ilcplex/ilocplex.h>
#include <string>
#include <vector>

class Solver
{
  public:
    Solver(const std::vector<Product>& products,
           const std::vector<Mineral>& mineral_limits,
           const std::vector<Area>& areas);
    void solve();

  private:
    void instantiateVariables();
    void declareConstraints();
    bool solveModel();
    void displaySolution();

    const std::vector<Product>& _products;
    const std::vector<Mineral>& _mineral_limits;
    const std::vector<Area>& _areas;

    IloEnv _env;
    IloModel _model;
    IloNumVarArray _qty_produced;
    IloArray<IloNumVarArray> _factories_in_area;
    IloCplex _cplex;
};
