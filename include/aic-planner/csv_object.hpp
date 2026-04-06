#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
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
    template <typename T>
    static void print_table(const std::vector<T>& objects,
                            std::ostream& os = std::cout)
    {
        if (objects.empty())
            return;

        const CSVObject& first = static_cast<const CSVObject&>(objects[0]);
        os << "\n--- " << first.get_title() << " ---\n";
        auto headers = first.get_headers();

        // Print header row
        for (const auto& h : headers)
        {
            os << std::left << std::setw(15)
               << (h.length() > 15 ? h.substr(0, 15) : h) << " | ";
        }
        os << "\n" << std::string(headers.size() * 18, '-') << "\n";

        // Print each object as a row
        for (const auto& obj : objects)
        {
            const CSVObject& base_obj = static_cast<const CSVObject&>(obj);
            auto values = base_obj.get_values();
            for (const auto& v : values)
            {
                os << std::left << std::setw(15)
                   << (v.length() > 15 ? v.substr(0, 15) : v) << " | ";
            }
            os << "\n";
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
            rows.push_back(parse_line(line));
        }
        return rows;
    }

  protected:
    static std::vector<std::string> parse_line(const std::string& line)
    {
        std::vector<std::string> result;
        std::stringstream ss(line);
        std::string item;
        while (std::getline(ss, item, ','))
        {
            const std::string whitespace = " \t\n\r";
            size_t first = item.find_first_not_of(whitespace);
            if (std::string::npos == first)
            {
                result.push_back("");
                continue;
            }
            size_t last = item.find_last_not_of(whitespace);
            result.push_back(item.substr(first, (last - first + 1)));
        }
        return result;
    }
};
