// All Rights Reserved
//
// Copyright (c) 2025, github.com/BIBlical33
//
// Getting and processing input data
//
// This software may not be modified, or used in any form without the explicit
// permission of the copyright holder. For permission requests, please contact:
// mag1str.kram@gmail.com

#include "include/read_input_data.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <vector>

#include "include/cybercafe_monitoring_system.h"

namespace {

using cybercafe_monitoring_system::TimePoint;
using cybercafe_monitoring_system::CybercafeMonitoringSystem::Event::Type::
    kIncoming;
using CybercafeMonitoringSystem =
    cybercafe_monitoring_system::CybercafeMonitoringSystem;
using Id = CybercafeMonitoringSystem::Event::Id;

// Reads event body
std::unique_ptr<CybercafeMonitoringSystem::Event> ParseEventBody(
    std::istringstream& iss, TimePoint event_time, int event_id) {
  switch (static_cast<Id>(event_id)) {
    case Id::k1: {
      std::string client_name;
      if (!(iss >> client_name))
        throw std::runtime_error("Invalid event param");

      return std::make_unique<CybercafeMonitoringSystem::ClientArrivedEvent>(
          event_time, client_name);
    } break;
    case Id::k2: {
      int table_id;
      std::string client_name;
      if (!(iss >> client_name >> table_id))
        throw std::runtime_error("Invalid event param");

      return std::make_unique<CybercafeMonitoringSystem::ClientSatAtTableEvent>(
          event_time, client_name, table_id,
          CybercafeMonitoringSystem::Event::Type::kIncoming);
    } break;
    case Id::k3: {
      std::string client_name;
      if (!(iss >> client_name))
        throw std::runtime_error("Invalid event param");

      return std::make_unique<CybercafeMonitoringSystem::ClientWaitingEvent>(
          event_time, client_name);
    } break;
    case Id::k4: {
      std::string client_name;
      if (!(iss >> client_name))
        throw std::runtime_error("Invalid event param");

      return std::make_unique<CybercafeMonitoringSystem::ClientLeftEvent>(
          event_time, client_name, kIncoming);
    } break;
    default:
      throw std::runtime_error(
          std::format("Invalid incoming id: {}", event_id));
  }
}

// Reads time in HH:MM format
TimePoint ParseTime(std::istringstream& iss) {
  std::string time_token;
  if (!(iss >> time_token))
    throw std::invalid_argument("Failed to read time token");

  if (time_token.size() != 5 or time_token[2] != ':')
    throw std::invalid_argument("Invalid time format (expected HH:MM)");

  if (!std::isdigit(time_token[0]) or !std::isdigit(time_token[1]) or
      !std::isdigit(time_token[3]) or !std::isdigit(time_token[4]))
    throw std::invalid_argument("Time contains non-digit characters");

  int hours = std::stoi(time_token.substr(0, 2)),
      minutes = std::stoi(time_token.substr(3, 2));

  if (hours < 0 or hours > 23)
    throw std::invalid_argument("Hours out of range (0-23)");
  else if (minutes < 0 or minutes > 59)
    throw std::invalid_argument("Minutes out of range (0-59)");

  return TimePoint(std::chrono::minutes{hours * 60 + minutes});
}

// Reads and validates CybercafeMonitoringSystem constructor arguments
CybercafeMonitoringSystem CreateTestObject(std::ifstream& file) {
  std::string file_line;
  std::getline(file, file_line);

  int cybercafe_tables_count = std::stoi(file_line);
  if (cybercafe_tables_count <= 0) throw std::runtime_error(file_line);

  std::getline(file, file_line);

  std::istringstream iss(file_line);
  TimePoint cybercafe_opening_time = ParseTime(iss);
  TimePoint cybercafe_closing_time = ParseTime(iss);

  std::getline(file, file_line);
  int cybercafe_pc_hourly_rate = std::stoi(file_line);
  if (cybercafe_pc_hourly_rate <= 0) throw std::runtime_error(file_line);

  return CybercafeMonitoringSystem(
      cybercafe_opening_time, cybercafe_closing_time, cybercafe_tables_count,
      cybercafe_pc_hourly_rate);
}

// Checks that the events are in the correct chronological order
void ValidateEventsOrder(
    std::span<std::unique_ptr<CybercafeMonitoringSystem::Event>> events) {
  if (events.size() > 1) {
    auto previous_event_time = events[0]->GetTime();
    for (size_t i = 1, iend = events.size(); i != iend; ++i)
      if (events[i]->GetTime() < previous_event_time) {
        events[i]->Print();
        std::exit(1);
      }
  }
}

}  // namespace

namespace cybercafe_monitoring_system_test {

// Reading CybercafeMonitoringSystem constructor arguments and events arguments
// from file. If some data is incorrect, returns first incorrect data line. To
// understand the order of arguments in file, see README.md
void ProcessingInputData(std::ifstream& file) {
  std::string file_line;

  try {
    cybercafe_monitoring_system::CybercafeMonitoringSystem test_object =
        CreateTestObject(file);
    std::vector<std::unique_ptr<CybercafeMonitoringSystem::Event>> test_events;

    while (std::getline(file, file_line)) {
      std::istringstream iss(file_line);

      TimePoint event_time = ParseTime(iss);

      int event_id;
      if (!(iss >> event_id)) throw std::runtime_error(file_line);

      test_events.push_back(ParseEventBody(iss, event_time, event_id));
    }

    ValidateEventsOrder(test_events);

    test_object.StartWorkDayTrigger();

    for (auto& ev : test_events) ev->Handle(test_object);

    test_object.EndWorkDayTrigger();
  } catch (const std::invalid_argument&) {
    throw std::runtime_error(file_line);
  } catch (const std::out_of_range&) {
    throw std::runtime_error(file_line);
  }
}

}  // namespace cybercafe_monitoring_system_test
