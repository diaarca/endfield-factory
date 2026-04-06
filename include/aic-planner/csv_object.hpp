#pragma once

#include "csv_reader.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class CSVObject
{
  public:
    virtual ~CSVObject() = default;

    // Returns the title of the table for this object type
    virtual std::string get_title() const = 0;

    // Returns the headers for the CSV columns
    virtual std::vector<std::string> get_headers() const = 0;

    // Returns the values for the CSV columns as strings
    virtual std::vector<std::string> get_values() const = 0;

    // Modular operator<< to print a single instance
    friend std::ostream& operator<<(std::ostream& os, const CSVObject& obj)
    {
        auto headers = obj.get_headers();
        auto values = obj.get_values();
        for (size_t i = 0; i < headers.size(); ++i)
        {
            os << std::left << std::setw(15)
               << (headers[i].length() > 15 ? headers[i].substr(0, 15)
                                            : headers[i])
               << ": "
               << (values[i].length() > 15 ? values[i].substr(0, 15)
                                           : values[i])
               << (i == headers.size() - 1 ? "" : " | ");
        }
        return os;
    }

    // Generic function to display a table of any objects inheriting from
    // CSVObject
    template <typename T> static void print_table(const std::vector<T>& objects)
    {
        if (objects.empty())
            return;

        // Use the first object to get the headers and title
        const CSVObject& first = static_cast<const CSVObject&>(objects[0]);
        std::cout << "\n--- " << first.get_title() << " ---\n";
        auto headers = first.get_headers();

        // Print header row
        for (const auto& h : headers)
        {
            std::cout << std::left << std::setw(15)
                      << (h.length() > 15 ? h.substr(0, 15) : h) << " | ";
        }
        std::cout << "\n" << std::string(headers.size() * 18, '-') << "\n";

        // Print each object as a row
        for (const auto& obj : objects)
        {
            const CSVObject& base_obj = static_cast<const CSVObject&>(obj);
            auto values = base_obj.get_values();
            for (const auto& v : values)
            {
                std::cout << std::left << std::setw(15)
                          << (v.length() > 15 ? v.substr(0, 15) : v) << " | ";
            }
            std::cout << "\n";
        }
    }

    // Generic function to write a collection of CSVObjects to a file
    template <typename T>
    static void write_table(const std::string& filename,
                            const std::vector<T>& objects)
    {
        if (objects.empty())
            return;

        std::ofstream file(filename);
        if (!file.is_open())
            return;

        const CSVObject& first = static_cast<const CSVObject&>(objects[0]);
        auto headers = first.get_headers();

        for (size_t i = 0; i < headers.size(); ++i)
        {
            file << headers[i] << (i == headers.size() - 1 ? "" : ",");
        }
        file << "\n";

        for (const auto& obj : objects)
        {
            const CSVObject& base_obj = static_cast<const CSVObject&>(obj);
            auto values = base_obj.get_values();
            for (size_t i = 0; i < values.size(); ++i)
            {
                file << values[i] << (i == values.size() - 1 ? "" : ",");
            }
            file << "\n";
        }
    }

    // Static helper to read lines from a CSV file
    static std::vector<std::vector<std::string>>
    read_file(const std::string& filename)
    {
        std::vector<std::vector<std::string>> rows;
        std::ifstream file(filename);
        if (!file.is_open())
        {
            return rows;
        }
        std::string line;
        while (std::getline(file, line))
        {
            rows.push_back(CSVReader::parse_line(line));
        }
        return rows;
    }
};
