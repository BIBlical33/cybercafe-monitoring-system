// All Rights Reserved
//
// Copyright (c) 2025, github.com/BIBlical33
//
// Cybercafe monitoring system
//
// This software may not be modified without the explicit permission of the
// copyright holder. For permission requests, please contact:
// mag1str.kram@gmail.com

#include "include/cybercafe_monitoring_system.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <deque>
#include <format>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

using cybercafe_monitoring_system::TimePoint;

// Prints time in HH:MM format
void PrintTimePoint(const auto& time_point) {
  auto dp = std::chrono::floor<std::chrono::days>(time_point);
  auto time = std::chrono::hh_mm_ss(time_point - dp);
  std::cout << std::format("{:%H:%M}", time);
}

// Prints minutes in HH:MM format
void PrintDurationAsHHMM(const std::chrono::minutes& duration) {
  auto total_hours = duration.count() / 60;
  auto total_mins = duration.count() % 60;

  std::cout << std::setfill('0') << std::setw(2) << total_hours << ":"
            << std::setw(2) << total_mins;
}

}  // namespace

namespace cybercafe_monitoring_system {

// Prints event header
void CybercafeMonitoringSystem::Event::Print() const {
  auto since_epoch = GetTime().time_since_epoch();

  auto hours = std::chrono::duration_cast<std::chrono::hours>(since_epoch);
  since_epoch -= hours;
  auto minutes = std::chrono::duration_cast<std::chrono::minutes>(since_epoch);

  std::cout << std::setfill('0') << std::setw(2) << hours.count() % 24 << ":"
            << std::setw(2) << minutes.count() % 60 << " "
            << static_cast<int>(GetId()) << " ";

  PrintEventBody();
}

void CybercafeMonitoringSystem::ClientArrivedEvent::Handle(
    CybercafeMonitoringSystem& system) {
  Print();

  if (system.clients_.contains(client_name_)) {
    ErrorEvent(GetTime(), "YouShallNotPass").Print();
    return;
  }

  if (not system.IsWorking(GetTime())) {
    ErrorEvent(GetTime(), "NotOpenYet").Print();
    return;
  }

  system.clients_.insert(client_name_);
}

void CybercafeMonitoringSystem::ClientSatAtTableEvent::Handle(
    CybercafeMonitoringSystem& system) {
  Print();

  switch (static_cast<Id>(id_)) {
    case Id::k2: {
      if (!system.IsTableFree(table_id_)) {
        ErrorEvent(GetTime(), "PlaceIsBusy").Print();
        return;
      }

      if (not system.clients_.contains(client_name_)) {
        ErrorEvent(GetTime(), "ClientUnknown").Print();
        return;
      }

      if (system.clients_at_table_.contains(client_name_)) {
        system.ProcessClientDeparture(client_name_, time_);
        system.clients_.insert(client_name_);
      }

      system.clients_at_table_[client_name_] = table_id_;
      system.tables_current_using_since_[table_id_] = time_;
    } break;

    case Id::k12: {
      system.clients_at_table_[client_name_] = table_id_;
      system.tables_current_using_since_[table_id_] = time_;
    } break;
    default:
      throw std::invalid_argument(
          std::format("Invalid event id {}", static_cast<int>(id_)));
  }
}

void CybercafeMonitoringSystem::ClientWaitingEvent::Handle(
    CybercafeMonitoringSystem& system) {
  Print();

  if (system.IsAvailableTableExists()) {
    ErrorEvent(GetTime(), "ICanWaitNoLonger!").Print();
    return;
  }

  if (system.clients_at_table_.contains(client_name_)) {
    ErrorEvent(GetTime(), "YouAlreadyAtTable!").Print();
    return;
  }

  if (static_cast<int>(system.waiting_clients_.size()) >=
      system.tables_count_) {
    ClientLeftEvent(GetTime(), client_name_, Event::Type::kOutgoing)
        .Handle(system);
    return;
  }

  if (not system.clients_.contains(client_name_)) {
    ErrorEvent(GetTime(), "ClientUnknown").Print();
    return;
  }

  system.waiting_clients_.push_back(client_name_);
}

void CybercafeMonitoringSystem::ClientLeftEvent::Handle(
    CybercafeMonitoringSystem& system) {
  Print();

  switch (static_cast<Id>(id_)) {
    case Id::k4: {
      if (not system.clients_.contains(client_name_)) {
        ErrorEvent(GetTime(), "ClientUnknown").Print();
        return;
      }

      if (not system.clients_at_table_.contains(client_name_)) {
        system.clients_.erase(client_name_);
        auto [erase_begin_it, erase_end_it] =
            std::ranges::remove(system.waiting_clients_, client_name_);
        system.waiting_clients_.erase(erase_begin_it, erase_end_it);
        return;
      }

      int table_id = system.clients_at_table_[client_name_];
      system.ProcessClientDeparture(client_name_, GetTime());

      if (not system.waiting_clients_.empty()) {
        ClientSatAtTableEvent(GetTime(), system.waiting_clients_.front(),
                              table_id, Event::Type::kOutgoing)
            .Handle(system);
        system.waiting_clients_.pop_front();
      }
    } break;
    case Id::k11: {
      if (not system.clients_at_table_.contains(client_name_)) {
        system.clients_.erase(client_name_);
        auto [erase_begin_it, erase_end_it] =
            std::ranges::remove(system.waiting_clients_, client_name_);
        system.waiting_clients_.erase(erase_begin_it, erase_end_it);
        return;
      }

      system.ProcessClientDeparture(client_name_, GetTime());
    } break;
    default:
      throw std::invalid_argument(
          std::format("Invalid event id {}", static_cast<int>(id_)));
  }
}

CybercafeMonitoringSystem::CybercafeMonitoringSystem(
    const TimePoint& opening_time, const TimePoint& closing_time,
    int tables_count, int hourly_rate)
    : hourly_rate_(hourly_rate),
      opening_time_(opening_time),
      closing_time_(closing_time),
      tables_count_(tables_count) {
  if (tables_count < 1)
    throw std::invalid_argument(
        std::format("Invalid tables count: {}", tables_count));
}

// Prints the desk number, its revenue for the day and the time it was
// occupied during the working day
void CybercafeMonitoringSystem::PrintClosingStats() const {
  PrintTimePoint(closing_time_);
  std::cout << std::endl;

  for (int table_id = 1; table_id < tables_count_; ++table_id) {
    std::cout << table_id << " " << tables_daily_revenue_.at(table_id) << " ";
    PrintDurationAsHHMM(tables_daily_using_.at(table_id));
    std::cout << std::endl;
  }

  std::cout << tables_count_ << " " << tables_daily_revenue_.at(tables_count_)
            << " ";
  PrintDurationAsHHMM(tables_daily_using_.at(tables_count_));
}

bool CybercafeMonitoringSystem::IsTableFree(int table_id) const {
  if (table_id < 1 or table_id > tables_count_)
    throw std::invalid_argument(
        std::format("Incorrect table id: {}", table_id));

  for (const auto& [client, table] : clients_at_table_)
    if (table == table_id) return false;

  return true;
}

// Calls when the cybercafe opens
void CybercafeMonitoringSystem::CybercafeOpen() {
  for (int i = 1; i <= tables_count_; ++i) {
    tables_daily_revenue_[i] = 0;
    tables_daily_using_[i] = std::chrono::minutes{0ll};
  }

  PrintTimePoint(opening_time_);
  std::cout << std::endl;
}

// Calls when the cybercafe closes
void CybercafeMonitoringSystem::CybercafeClose() {
  std::vector<std::string> remaining_clients;
  for (const auto& client : clients_) remaining_clients.push_back(client);

  std::ranges::sort(remaining_clients, ClientsNameCompare{});

  for (const auto& client : remaining_clients) {
    ClientLeftEvent(closing_time_, client, Event::Type::kOutgoing)
        .Handle(*this);
  }

  PrintClosingStats();

  tables_daily_using_.clear();
  tables_current_using_since_.clear();
  tables_daily_revenue_.clear();
}

// Deletes client from database
void CybercafeMonitoringSystem::ProcessClientDeparture(
    const std::string& client_name, const TimePoint& time) {
  int table_id = clients_at_table_.at(client_name);

  auto usage_duration = time - tables_current_using_since_.at(table_id);
  tables_daily_using_[table_id] += usage_duration;

  int64_t hours = (static_cast<int64_t>(usage_duration.count()) + 59) / 60;
  tables_daily_revenue_[table_id] += hours * hourly_rate_;
  total_revenue_ += hours * hourly_rate_;

  clients_at_table_.erase(client_name);
  clients_.erase(client_name);
  tables_current_using_since_.erase(table_id);
}

}  // namespace cybercafe_monitoring_system
