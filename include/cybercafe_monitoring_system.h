// All Rights Reserved
//
// Copyright (c) 2025, github.com/BIBlical33
//
// Cybercafe monitoting system
//
// This software may not be modified without the explicit permission of the
// copyright holder. For permission requests, please contact:
// mag1str.kram@gmail.com

#ifndef INCLUDE_CYBERCAFE_MONITORING_SYSTEM_H_
#define INCLUDE_CYBERCAFE_MONITORING_SYSTEM_H_

#include <algorithm>
#include <chrono>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace cybercafe_monitoring_system {

using TimePoint =
    std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;

class CybercafeMonitoringSystem final {
 public:
  class Event {
   public:
    enum class Id {
      k1 = 1,
      k2 = 2,
      k3 = 3,
      k4 = 4,
      k11 = 11,
      k12 = 12,
      k13 = 13,
      kBadId,
    };

    enum class Type {
      kIncoming,
      kOutgoing,
    };

    virtual ~Event() = default;

    virtual void Handle(CybercafeMonitoringSystem& system) = 0;

    // Prints event header
    void Print() const;

    inline TimePoint GetTime() const { return time_; }

    inline Id GetId() const { return id_; }

    inline Type GetType() const { return type_; }

   protected:
    Event(const TimePoint& time, Id event_id, Type event_type)
        : time_(time), id_(event_id), type_{event_type} {}

    inline static bool IsClientNameValid(std::string_view client_name) {
      for (char c : client_name)
        if (not(std::isdigit(c) or std::islower(c) or c == '_' or c == '-'))
          return false;

      return !client_name.empty();
    }

    // Prints event body
    virtual void PrintEventBody() const = 0;

    TimePoint time_;

    Id id_ = Id::kBadId;

    Type type_;
  };

  class ClientArrivedEvent final : public Event {
   public:
    ClientArrivedEvent(const TimePoint& event_time,
                       std::string_view client_name)
        : Event(event_time, Id::k1, Type::kIncoming),
          client_name_(client_name) {
      if (not IsClientNameValid(client_name))
        throw std::invalid_argument(
            std::format("Invalid client name: {}", client_name));
    }

    void Handle(CybercafeMonitoringSystem& system) override;

    inline std::string GetClientName() const { return client_name_; }

   private:
    inline void PrintEventBody() const override {
      std::cout << client_name_ << std::endl;
    }

    std::string client_name_;
  };

  class ClientSatAtTableEvent final : public Event {
   public:
    ClientSatAtTableEvent(const TimePoint& event_time,
                          std::string_view client_name, int table_id,
                          Type event_type)
        : Event(event_time, (event_type == Type::kIncoming ? Id::k2 : Id::k12),
                event_type),
          client_name_(client_name),
          table_id_(table_id) {
      if (not IsClientNameValid(client_name))
        throw std::invalid_argument(
            std::format("Invalid client name: {}", client_name));
    }

    void Handle(CybercafeMonitoringSystem& system) override;

    inline std::string GetClientName() const { return client_name_; }

    inline int GetTableNum() const { return table_id_; }

   private:
    inline void PrintEventBody() const override {
      std::cout << client_name_ << " " << table_id_ << std::endl;
    }

    std::string client_name_;

    int table_id_;
  };

  class ClientWaitingEvent final : public Event {
   public:
    ClientWaitingEvent(const TimePoint& event_time,
                       std::string_view client_name)
        : Event(event_time, Id::k3, Type::kIncoming),
          client_name_(client_name) {
      if (not IsClientNameValid(client_name))
        throw std::invalid_argument(
            std::format("Invalid client name: {}", client_name));
    }

    void Handle(CybercafeMonitoringSystem& system) override;

    inline std::string GetClientName() const { return client_name_; }

   private:
    inline void PrintEventBody() const override {
      std::cout << client_name_ << std::endl;
    }

    std::string client_name_;
  };

  class ClientLeftEvent final : public Event {
   public:
    ClientLeftEvent(const TimePoint& event_time, std::string_view client_name,
                    Type event_type)
        : Event(event_time, (event_type == Type::kIncoming ? Id::k4 : Id::k11),
                event_type),
          client_name_(client_name) {
      if (not IsClientNameValid(client_name))
        throw std::invalid_argument(
            std::format("Invalid client name: {}", client_name));
    }

    void Handle(CybercafeMonitoringSystem& system) override;

    inline std::string GetClientName() const { return client_name_; }

   private:
    inline void PrintEventBody() const override {
      std::cout << client_name_ << std::endl;
    }

    std::string client_name_;
  };

  class ErrorEvent final : public Event {
   public:
    ErrorEvent(const TimePoint& event_time, std::string_view error_message)
        : Event(event_time, Id::k13, Type::kOutgoing),
          error_message_(error_message) {}

    inline void Handle(CybercafeMonitoringSystem&) override { Print(); };

    inline std::string What() const { return error_message_; }

   private:
    inline void PrintEventBody() const override {
      std::cout << error_message_ << std::endl;
    }

    std::string error_message_;
  };

  CybercafeMonitoringSystem(const TimePoint& opening_time,
                            const TimePoint& closing_time, int tables_count,
                            int hourly_rate);

  // Remove this if you plan to modify the prototype for real-time use
  inline void StartWorkDayTrigger() { CybercafeOpen(); };

  // Remove this if you plan to modify the prototype for real-time use
  inline void EndWorkDayTrigger() { CybercafeClose(); };

  // Prints the desk number, its revenue for the day and the time it was
  // occupied during the working day
  void PrintClosingStats() const;

  inline bool IsWorking(const TimePoint& time) const {
    return time >= opening_time_ and time < closing_time_;
  }

  inline bool IsAvailableTableExists() const {
    return clients_at_table_.size() < static_cast<size_t>(tables_count_);
  }

  bool IsTableFree(int table_id) const;

#if 0
  // For future

  void SetTablesCount() const;

  void SetOpeningTime() const;

  void SetClosingTime() const;

#endif
  inline int64_t GetTotalRevenue() const { return total_revenue_; }

  int hourly_rate_;

 private:
  // For sorting clients names
  class ClientsNameCompare final {
   public:
    bool operator()(std::string_view first, std::string_view second) const {
      if (first == second) return false;

      for (size_t i = 0, iend = std::min(first.size(), second.size());
           i != iend; ++i) {
        const int first_rank = CharacterRank(first[i]),
                  second_rank = CharacterRank(second[i]);
        if (first_rank != second_rank) return first_rank < second_rank;
      }

      return first.size() < second.size();
    }

   private:
    static int CharacterRank(char c) {
      if (c >= 'a' and c <= 'z')
        return c - 'a';
      else if (c >= '0' and c <= '9')
        return 26 + (c - '0');
      else if (c == '_')
        return 36;
      else if (c == '-')
        return 37;

      throw std::runtime_error(
          std::format("Invalid character in client name: {}", c));
    }
  };

  friend ClientArrivedEvent;
  friend ClientLeftEvent;
  friend ClientSatAtTableEvent;
  friend ClientWaitingEvent;

  // Calls when cybercafe opening
  void CybercafeOpen();

  // Calls when cybercafe opening
  void CybercafeClose();

  // Deletes client from database
  void ProcessClientDeparture(const std::string& client_name,
                              const TimePoint& time);

  TimePoint opening_time_, closing_time_;

  int tables_count_;

  int64_t total_revenue_ = 0;

  std::deque<std::string> waiting_clients_{};

  // You can ñhange it into a database
  std::unordered_set<std::string> clients_{};

  // You can ñhange it into a database
  std::unordered_map<std::string, int> clients_at_table_{};

  // You can ñhange it into a database
  std::unordered_map<int, TimePoint> tables_current_using_since_;

  // You can ñhange it into a database
  std::unordered_map<int, std::chrono::minutes> tables_daily_using_;

  // You can ñhange it into a database
  std::unordered_map<int, int64_t> tables_daily_revenue_;
};

}  // namespace cybercafe_monitoring_system

#endif  // INCLUDE_CYBERCAFE_MONITORING_SYSTEM_H_
