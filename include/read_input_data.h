// All Rights Reserved
//
// Copyright (c) 2025, github.com/BIBlical33
//
// Reading input data from a file
//
// This software may not be modified without the explicit permission of the
// copyright holder. For permission requests, please contact:
// mag1str.kram@gmail.com

#ifndef INCLUDE_READ_INPUT_DATA_H_
#define INCLUDE_READ_INPUT_DATA_H_

#include <fstream>

namespace cybercafe_monitoring_system_test {

// Reading CybercafeMonitoringSystem constructor arguments and events arguments
// from file. To understand the order of arguments in file, see README.md
void ProcessingInputData(std::ifstream& file);

}  // namespace cybercafe_monitoring_system_test

#endif  // INCLUDE_READ_INPUT_DATA_H_
