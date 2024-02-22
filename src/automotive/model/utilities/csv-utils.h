#ifndef CSV_UTILS_H
#define CSV_UTILS_H

#include <string>
#include <fstream>
#include <unistd.h>

namespace ns3 {
  template <typename... Args>
  void
  writeDataToCSV (std::string filepath, std::string header, Args... args)
  {
    std::ofstream csv_out;
    bool first_element = true;

    if (access (filepath.c_str (), F_OK) != -1)
      {
        // The file already exists
        csv_out.open (filepath, std::ofstream::out | std::ofstream::app);
      }
    else
      {
        // The file does not exist yet
        csv_out.open (filepath);
        csv_out << header << std::endl;
      }

    // Lambda to print an element with a preceding comma if it is not the first element
    auto print_csv_arg = [&first_element, &csv_out] (const auto &arg) {
      if (!first_element)
        {
          csv_out << ",";
        }
      else
        {
          first_element = false;
        }
      csv_out << arg;
    };

    // Fold expression to save to the CSV file all the specified data
    (..., print_csv_arg (args));

    csv_out << std::endl;
  }
}

#endif // CSV_UTILS_H