#include "DataReader.h"
#include "Solver.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <data_folder_path>" << std::endl;
        return 1;
    }
    std::string folder_path = argv[1];

    try
    {
        // Read data
        std::string minerals_file = folder_path + "/minerals.csv";
        std::string products_file = folder_path + "/products.csv";

        auto mineral_limits = DataReader::read_minerals(minerals_file);
        auto products = DataReader::read_products(products_file);

        if (products.empty() || mineral_limits.empty())
        {
            std::cerr << "Data files are empty or could not be read properly."
                      << std::endl;
            return 1;
        }

        // Create solver and solve the model
        Solver solver;
        solver.solve(products, mineral_limits);
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nAn error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
