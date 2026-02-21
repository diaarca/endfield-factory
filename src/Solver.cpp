#include "Solver.h"
#include <ilcplex/ilocplex.h>
#include <iostream>

void Solver::solve(const std::vector<Product>& products,
                   const std::map<std::string, double>& mineral_limits)
{
    IloEnv env;
    try
    {
        IloModel model(env);

        IloNumVarArray qty_produced(env, products.size(), 0, IloInfinity,
                                    ILOINT);
        for (size_t i = 0; i < products.size(); ++i)
        {
            qty_produced[i].setName(products[i].name.c_str());
        }

        IloExpr objective(env);
        for (size_t i = 0; i < products.size(); ++i)
        {
            objective += products[i].value * qty_produced[i];
        }
        model.add(IloMaximize(env, objective));
        objective.end();

        for (const auto& pair : mineral_limits)
        {
            const std::string& mineral_name = pair.first;
            double mineral_limit = pair.second;

            IloExpr mineral_consumption_expr(env);
            for (size_t i = 0; i < products.size(); ++i)
            {
                if (products[i].mineral_consumption.count(mineral_name))
                {
                    mineral_consumption_expr +=
                        products[i].mineral_consumption.at(mineral_name) *
                        qty_produced[i];
                }
            }
            IloConstraint con = mineral_consumption_expr <= mineral_limit;
            con.setName(mineral_name.c_str());
            model.add(con);
            mineral_consumption_expr.end();
        }

        IloCplex cplex(model);
        // cplex.exportModel("model.lp");
        cplex.setOut(env.getNullStream());

        if (cplex.solve())
        {
            std::cout << "Solution Status: " << cplex.getStatus() << std::endl;
            std::cout
                << "Optimal Objective Value (Maximized Value per Minute): "
                << cplex.getObjValue() << std::endl;
            std::cout << "\n--- Production Plan (units per minute) ---"
                      << std::endl;

            for (size_t i = 0; i < products.size(); ++i)
            {
                double quantity = cplex.getValue(qty_produced[i]);
                if (quantity > 1e-6)
                {
                    std::cout << products[i].name << ": " << quantity
                              << " units" << std::endl;
                }
            }

            std::cout << "\n--- Mineral Consumption (usage / limit) ---"
                      << std::endl;
            for (const auto& pair : mineral_limits)
            {
                const std::string& mineral_name = pair.first;
                double mineral_limit = pair.second;

                double total_consumed = 0.0;
                for (size_t i = 0; i < products.size(); ++i)
                {
                    if (products[i].mineral_consumption.count(mineral_name))
                    {
                        total_consumed +=
                            products[i].mineral_consumption.at(mineral_name) *
                            cplex.getValue(qty_produced[i]);
                    }
                }
                std::cout << mineral_name << ": " << total_consumed << " / "
                          << mineral_limit << std::endl;
            }
        }
        else
        {
            std::cerr << "No solution found." << std::endl;
        }
    }
    catch (const IloException& e)
    {
        std::cerr << "\nConcert exception caught: " << e << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nStandard exception caught: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "\nUnknown exception caught" << std::endl;
    }

    env.end();
}
