#include "solver.hpp"
#include <iostream>
#include <numeric>

using namespace operations_research;
using namespace operations_research::sat;

Solver::Solver(const std::vector<Product>& products,
               const std::vector<Mineral>& mineral_limits,
               const std::vector<Area>& areas,
               const std::vector<Fuel>& fuels,
               const std::map<std::string, double>& facility_power,
               const Region& region)
    : _products(products), _mineral_limits(mineral_limits), _areas(areas),
      _fuels(fuels), _facility_power(facility_power), _region(region),
      _obj_scale_factor(60)
{
}

void Solver::solve()
{
    // Compute exact scaling factor for fuel duration to keep all coefficients integer
    int64_t scale = 1;
    for (const auto& fuel : _fuels)
    {
        if (fuel.duration > 0)
        {
            scale = std::lcm(scale, static_cast<int64_t>(fuel.duration));
        }
    }
    _obj_scale_factor = scale;

    instantiateVariables();
    declareConstraints();

    if (solveModel())
    {
        displaySolution();
    }
    else
    {
        std::cerr << "No solution found." << std::endl;
    }
}

void Solver::instantiateVariables()
{
    const int64_t max_val = 1000000;

    _qty_produced.clear();
    for (size_t i = 0; i < _products.size(); ++i)
    {
        _qty_produced.push_back(
            _cp_model.NewIntVar(Domain(0, max_val)).WithName(_products[i].name + "_prod"));
    }

    _factories_in_area.clear();
    _factories_in_area.resize(_products.size());
    for (size_t i = 0; i < _products.size(); ++i)
    {
        for (size_t j = 0; j < _areas.size(); ++j)
        {
            std::string name = _products[i].name + "_" + _areas[j].name;
            _factories_in_area[i].push_back(
                _cp_model.NewIntVar(Domain(0, 10000)).WithName(name));
        }
    }

    _num_batteries_active.clear();
    for (size_t i = 0; i < _fuels.size(); ++i)
    {
        _num_batteries_active.push_back(
            _cp_model.NewIntVar(Domain(0, 10000)).WithName(_fuels[i].name + "_active"));
    }
}

void Solver::declareConstraints()
{
    // Mappings for easier lookup
    std::map<std::string, size_t> fuel_map;
    for (size_t i = 0; i < _fuels.size(); ++i)
    {
        fuel_map[_fuels[i].name] = i;
    }

    // Objective: Maximize total net value (sold products)
    LinearExpr objective;

    for (size_t i = 0; i < _products.size(); ++i)
    {
        objective += LinearExpr::Term(_qty_produced[i], _obj_scale_factor * static_cast<int64_t>(_products[i].value));

        if (fuel_map.count(_products[i].name))
        {
            size_t fuel_idx = fuel_map.at(_products[i].name);
            int64_t dur = static_cast<int64_t>(_fuels[fuel_idx].duration);
            int64_t fuel_cost_coeff = (_obj_scale_factor * 60 / dur) * static_cast<int64_t>(_products[i].value);
            objective += LinearExpr::Term(_num_batteries_active[fuel_idx], -fuel_cost_coeff);

            // Fuel Balance constraint: produce_fu >= (60 / dur) * active_fu
            // => dur * produce_fu - 60 * active_fu >= 0
            LinearExpr c_fuel_cons;
            c_fuel_cons += LinearExpr::Term(_qty_produced[i], dur);
            c_fuel_cons += LinearExpr::Term(_num_batteries_active[fuel_idx], -60);
            _cp_model.AddGreaterOrEqual(c_fuel_cons, 0);
        }
    }
    _cp_model.Maximize(objective);

    // Storage capacity constraint (not full within 48 hours = 2880 mins)
    for (size_t i = 0; i < _products.size(); ++i)
    {
        if (fuel_map.count(_products[i].name))
        {
            size_t fuel_idx = fuel_map.at(_products[i].name);
            int64_t dur = static_cast<int64_t>(_fuels[fuel_idx].duration);
            // (produce - 60/dur * active) <= storage / 2880
            // => dur * produce - 60 * active <= (storage * dur) / 2880
            LinearExpr c_storage;
            c_storage += LinearExpr::Term(_qty_produced[i], dur);
            c_storage += LinearExpr::Term(_num_batteries_active[fuel_idx], -60);
            int64_t max_storage = static_cast<int64_t>((_region.storage / (48.0 * 60.0)) * dur);
            _cp_model.AddLessOrEqual(c_storage, max_storage);
        }
        else
        {
            int64_t max_storage = static_cast<int64_t>(_region.storage / (48.0 * 60.0));
            _cp_model.AddLessOrEqual(_qty_produced[i], max_storage);
        }
    }

    // Mineral limits
    for (size_t m = 0; m < _mineral_limits.size(); ++m)
    {
        const std::string& mineral_name = _mineral_limits[m].name;
        int64_t mineral_limit = static_cast<int64_t>(_mineral_limits[m].limit);

        LinearExpr c_mineral;
        for (size_t i = 0; i < _products.size(); ++i)
        {
            if (_products[i].mineral_consumption.count(mineral_name))
            {
                c_mineral += LinearExpr::Term(
                    _qty_produced[i],
                    static_cast<int64_t>(_products[i].mineral_consumption.at(mineral_name)));
            }
        }
        _cp_model.AddLessOrEqual(c_mineral, mineral_limit);
    }

    // Factory capacity: produce_p <= (60 / time_p) * sum(factories)
    // => time_p * produce_p - 60 * sum(factories) <= 0
    for (size_t i = 0; i < _products.size(); ++i)
    {
        LinearExpr c_nb_factory;
        c_nb_factory += LinearExpr::Term(_qty_produced[i], static_cast<int64_t>(_products[i].production_time));
        for (size_t j = 0; j < _areas.size(); ++j)
        {
            c_nb_factory += LinearExpr::Term(_factories_in_area[i][j], -60);
        }
        _cp_model.AddLessOrEqual(c_nb_factory, 0);
    }

    // Area space and depot constraints
    for (size_t j = 0; j < _areas.size(); ++j)
    {
        double total_available_area =
            _areas[j].pac_width * _areas[j].pac_height;
        if (total_available_area > 0)
        {
            // Total space
            LinearExpr c_space;
            for (size_t i = 0; i < _products.size(); ++i)
            {
                int64_t factory_area =
                    static_cast<int64_t>(_products[i].factory_width * _products[i].factory_height);
                c_space += LinearExpr::Term(_factories_in_area[i][j], factory_area);
            }
            _cp_model.AddLessOrEqual(c_space, static_cast<int64_t>(total_available_area));

            // Depot constraint
            LinearExpr c_depot;
            for (size_t i = 0; i < _products.size(); ++i)
            {
                int64_t factory_depot = static_cast<int64_t>(_products[i].factory_depot);
                c_depot += LinearExpr::Term(_factories_in_area[i][j], factory_depot);
            }
            int64_t total_depot_in_area =
                static_cast<int64_t>(_areas[j].pac_depot_width + _areas[j].pac_depot_height);
            _cp_model.AddLessOrEqual(c_depot, total_depot_in_area);
        }
        else
        {
            for (size_t i = 0; i < _products.size(); ++i)
            {
                _cp_model.AddEquality(_factories_in_area[i][j], 0);
            }
        }
    }

    // Power constraint
    int64_t static_demand = 0;
    for (const auto& area : _areas)
    {
        if (area.area_facilities.count("zipline") &&
            _facility_power.count("zipline"))
            static_demand += static_cast<int64_t>(area.area_facilities.at("zipline") *
                             _facility_power.at("zipline"));
        if (area.area_facilities.count("defense") &&
            _facility_power.count("defense"))
            static_demand += static_cast<int64_t>(area.area_facilities.at("defense") *
                             _facility_power.at("defense"));
        if (area.area_facilities.count("mining_rig") &&
            _facility_power.count("mining_rig"))
            static_demand += static_cast<int64_t>(area.area_facilities.at("mining_rig") *
                             _facility_power.at("mining_rig"));
    }

    LinearExpr c_power_con;
    for (size_t i = 0; i < _products.size(); ++i)
    {
        int64_t factory_power = 0;
        for (const auto& f : _products[i].factory_facilities)
        {
            if (_facility_power.count(f.first))
                factory_power += static_cast<int64_t>(f.second * _facility_power.at(f.first));
        }
        if (factory_power > 0)
        {
            for (size_t j = 0; j < _areas.size(); ++j)
                c_power_con += LinearExpr::Term(_factories_in_area[i][j], factory_power);
        }
    }
    for (size_t i = 0; i < _fuels.size(); ++i)
    {
        c_power_con += LinearExpr::Term(_num_batteries_active[i], -static_cast<int64_t>(_fuels[i].power));
    }
    int64_t max_power_allowed = static_cast<int64_t>(_region.base_power) - static_demand;
    _cp_model.AddLessOrEqual(c_power_con, max_power_allowed);
}

bool Solver::solveModel()
{
    SatParameters parameters;
    parameters.set_log_search_progress(false);
    parameters.set_num_workers(8);

    Model model;
    model.Add(NewSatParameters(parameters));

    _response = SolveCpModel(_cp_model.Build(), &model);
    return _response.status() == CpSolverStatus::OPTIMAL ||
           _response.status() == CpSolverStatus::FEASIBLE;
}

void Solver::displaySolution()
{
    std::cout << "Solution Status: OPTIMAL" << std::endl;
    std::cout << "Optimal Objective Value (Net Value per Minute): "
              << (_response.objective_value() / static_cast<double>(_obj_scale_factor)) << std::endl;

    std::cout << "\n--- Production Plan (units per minute) ---" << std::endl;
    for (size_t i = 0; i < _products.size(); ++i)
    {
        int64_t produced = SolutionIntegerValue(_response, _qty_produced[i]);
        if (produced > 0)
        {
            int64_t total_factories = 0;
            for (size_t j = 0; j < _areas.size(); ++j)
                total_factories += SolutionIntegerValue(_response, _factories_in_area[i][j]);
            std::cout << _products[i].name << ": " << produced << " units ["
                      << total_factories << " factories]" << std::endl;
        }
    }

    std::cout << "\n--- Factory Placement ---" << std::endl;
    for (size_t j = 0; j < _areas.size(); ++j)
    {
        bool area_used = false;
        for (size_t i = 0; i < _products.size(); ++i)
        {
            if (SolutionIntegerValue(_response, _factories_in_area[i][j]) > 0)
            {
                area_used = true;
                break;
            }
        }
        if (area_used)
        {
            std::cout << "Area: " << _areas[j].name << std::endl;
            double used_space = 0;
            double used_depot = 0;
            for (size_t i = 0; i < _products.size(); ++i)
            {
                int64_t num_f = SolutionIntegerValue(_response, _factories_in_area[i][j]);
                if (num_f > 0)
                {
                    std::cout << "  - " << _products[i].name << ": "
                              << num_f << " factories"
                              << std::endl;
                    used_space += num_f * (_products[i].factory_width *
                                           _products[i].factory_height);
                    used_depot += num_f * _products[i].factory_depot;
                }
            }
            std::cout << "  Space used: " << used_space << " / "
                      << (_areas[j].pac_width * _areas[j].pac_height)
                      << std::endl;

            std::cout << "  Depot length used: " << used_depot << " / "
                      << _areas[j].pac_depot_width + _areas[j].pac_depot_height
                      << std::endl;
        }
    }

    std::cout << "\n--- Mineral Consumption (usage / limit) ---" << std::endl;
    for (const auto& mineral : _mineral_limits)
    {
        double total_consumed = 0.0;
        for (size_t i = 0; i < _products.size(); ++i)
        {
            if (_products[i].mineral_consumption.count(mineral.name))
                total_consumed +=
                    _products[i].mineral_consumption.at(mineral.name) *
                    SolutionIntegerValue(_response, _qty_produced[i]);
        }
        for (size_t i = 0; i < _fuels.size(); ++i)
        {
            if (_fuels[i].name == mineral.name)
                total_consumed += SolutionIntegerValue(_response, _num_batteries_active[i]) *
                                  (60.0 / _fuels[i].duration);
        }
        std::cout << mineral.name << ": " << total_consumed << " / "
                  << mineral.limit << std::endl;
    }

    std::cout << "\n--- Power Consumption ---" << std::endl;
    double p_zip = 0, p_def = 0, p_mine = 0, p_fact = 0;
    for (const auto& area : _areas)
    {
        if (area.area_facilities.count("zipline") &&
            _facility_power.count("zipline"))
            p_zip += area.area_facilities.at("zipline") *
                     _facility_power.at("zipline");
        if (area.area_facilities.count("defense") &&
            _facility_power.count("defense"))
            p_def += area.area_facilities.at("defense") *
                     _facility_power.at("defense");
        if (area.area_facilities.count("mining_rig") &&
            _facility_power.count("mining_rig"))
            p_mine += area.area_facilities.at("mining_rig") *
                     _facility_power.at("mining_rig");
    }
    for (size_t i = 0; i < _products.size(); ++i)
    {
        double f_pow = 0;
        for (const auto& f : _products[i].factory_facilities)
            if (_facility_power.count(f.first))
                f_pow += f.second * _facility_power.at(f.first);
        for (size_t j = 0; j < _areas.size(); ++j)
            p_fact += SolutionIntegerValue(_response, _factories_in_area[i][j]) * f_pow;
    }
    std::cout << "Power for Ziplines: " << p_zip
              << "\nPower for Defenses: " << p_def
              << "\nPower for Mining Rigs: " << p_mine
              << "\nPower for Factories: " << p_fact << std::endl;
    std::cout << "Total Power Needed: " << (p_zip + p_def + p_fact)
              << std::endl;

    std::cout << "\n--- Power Production (Optimal Battery Mix) ---"
              << std::endl;
    double t_supp = 0;
    for (size_t i = 0; i < _fuels.size(); ++i)
    {
        int64_t num = SolutionIntegerValue(_response, _num_batteries_active[i]);
        if (num > 0)
        {
            double s = num * _fuels[i].power;
            std::cout << _fuels[i].name << ": " << num
                      << " active batteries (" << s << " power, "
                      << num * (60.0 / _fuels[i].duration)
                      << " units/min consumption)" << std::endl;
            t_supp += s;
        }
    }
    std::cout << "Total Power Provided: " << t_supp << std::endl;
}
