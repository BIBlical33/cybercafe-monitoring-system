// All Rights Reserved
//
// Copyright (c) 2025, github.com/BIBlical33
//
// Launching a console application
//
// This software may not be modified, or used in any form without the explicit
// permission of the copyright holder. For permission requests, please contact:
// mag1str.kram@gmail.com

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "include/read_input_data.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: <target filename> <filename of file for reading the "
                 "input data>\n";
    return 1;
  }

  std::filesystem::path current_path = std::filesystem::current_path();

  while (current_path.has_parent_path()) {
    if (current_path.filename() == "cybercafe-monitoring-system") {
      break;
    }
    current_path = current_path.parent_path();
  }

  std::filesystem::path file_path = current_path / "tests" / argv[1];

  if (!std::filesystem::exists(file_path)) {
    std::cerr << "File not found: " << file_path << std::endl;
    return 1;
  }

  std::ifstream file(file_path);
  if (!file.is_open()) {
    std::cerr << "Cannot open file: " << argv[1];
    return 1;
  }

  try {
    cybercafe_monitoring_system_test::ProcessingInputData(file);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what();
    return 1;
  }
}
