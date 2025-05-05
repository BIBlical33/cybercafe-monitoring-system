// All Rights Reserved
//
// Copyright (c) 2025, github.com/BIBlical33
//
// Main cybercafe monitoring system test
//
// This software may not be modified without the explicit permission of the
// copyright holder. For permission requests, please contact:
// mag1str.kram@gmail.com

#include <gtest/gtest.h>

#include <chrono>

#include "include/cybercafe_monitoring_system.h"

namespace {

using cybercafe_monitoring_system::CybercafeMonitoringSystem;
using cybercafe_monitoring_system::TimePoint;
using std::chrono::minutes;

using Event = CybercafeMonitoringSystem::Event;
using EventId = CybercafeMonitoringSystem::Event::Id;
using EventType = CybercafeMonitoringSystem::Event::Type;

using ClientArrivedEvent = CybercafeMonitoringSystem::ClientArrivedEvent;
using ClientSatAtTableEvent = CybercafeMonitoringSystem::ClientSatAtTableEvent;
using ClientWaitingEvent = CybercafeMonitoringSystem::ClientWaitingEvent;
using ClientLeftEvent = CybercafeMonitoringSystem::ClientLeftEvent;
using ErrorEvent = CybercafeMonitoringSystem::ErrorEvent;

class CybercafeMonitoringSystemTest : public ::testing::Test {
 protected:
  TimePoint opening_time = TimePoint{minutes{10 * 60}};  // 10:00
  TimePoint closing_time = TimePoint{minutes{22 * 60}};  // 22:00
  int tables_count = 3;
  int hourly_rate = 10;

  void SetUp() override {
    system = std::make_unique<CybercafeMonitoringSystem>(
        opening_time, closing_time, tables_count, hourly_rate);
    system->StartWorkDayTrigger();
  }

  std::unique_ptr<CybercafeMonitoringSystem> system;
};

TEST_F(CybercafeMonitoringSystemTest, ClientArrivalAndSitting) {
  TimePoint event_time = TimePoint{minutes{11 * 60}};  // 11:00

  ClientArrivedEvent arrive_event(event_time, "client1");
  arrive_event.Handle(*system);

  ClientSatAtTableEvent sit_event(event_time, "client1", 1,
                                  Event::Type::kIncoming);
  sit_event.Handle(*system);

  EXPECT_TRUE(system->IsWorking(event_time));
  EXPECT_FALSE(system->IsTableFree(1));
}

TEST_F(CybercafeMonitoringSystemTest, ClientWaitingQueue) {
  TimePoint event_time = TimePoint{minutes{12 * 60}};  // 12:00

  for (int i = 1; i <= tables_count; ++i) {
    std::string client = "client" + std::to_string(i);
    ClientArrivedEvent arrive_event(event_time, client);
    arrive_event.Handle(*system);

    ClientSatAtTableEvent sit_event(event_time, client, i,
                                    Event::Type::kIncoming);
    sit_event.Handle(*system);
  }

  for (int i = 1; i <= tables_count; ++i) {
    EXPECT_FALSE(system->IsTableFree(i));
  }

  ClientArrivedEvent arrive_event(event_time, "waiting_client");
  arrive_event.Handle(*system);

  ClientWaitingEvent wait_event(event_time, "waiting_client");
  wait_event.Handle(*system);

  system->EndWorkDayTrigger();
}

TEST_F(CybercafeMonitoringSystemTest, ClientDepartureAndRevenueCalculation) {
  TimePoint arrive_time = TimePoint{minutes{13 * 60}};  // 13:00
  TimePoint depart_time = TimePoint{minutes{14 * 60}};  // 14:00

  ClientArrivedEvent arrive_event(arrive_time, "client1");
  arrive_event.Handle(*system);

  ClientSatAtTableEvent sit_event(arrive_time, "client1", 1,
                                  Event::Type::kIncoming);
  sit_event.Handle(*system);

  ClientLeftEvent leave_event(depart_time, "client1", Event::Type::kIncoming);
  leave_event.Handle(*system);

  EXPECT_GT(system->GetTotalRevenue(), 0);
}

TEST_F(CybercafeMonitoringSystemTest, InvalidClientNameThrows) {
  TimePoint event_time = TimePoint{minutes{11 * 60}};

  EXPECT_THROW(ClientArrivedEvent(event_time, "Invalid Client"),
               std::invalid_argument);
  EXPECT_THROW(ClientSatAtTableEvent(event_time, "Invalid@Client", 1,
                                     Event::Type::kIncoming),
               std::invalid_argument);
  EXPECT_THROW(ClientWaitingEvent(event_time, "Client with spaces"),
               std::invalid_argument);
  EXPECT_THROW(ClientLeftEvent(event_time, "", Event::Type::kIncoming),
               std::invalid_argument);
}

TEST_F(CybercafeMonitoringSystemTest, ClientArrivalErrors) {
  TimePoint before_open = TimePoint{minutes{9 * 60}};  // 09:00
  TimePoint after_open = TimePoint{minutes{11 * 60}};  // 11:00

  ClientArrivedEvent before_open_event(before_open, "client1");
  before_open_event.Handle(*system);  // Should generate "NotOpenYet"

  ClientArrivedEvent first_arrival(after_open, "client2");
  first_arrival.Handle(*system);

  ClientArrivedEvent second_arrival(after_open, "client2");
  second_arrival.Handle(*system);  // Should generate "YouShallNotPass"
}

TEST_F(CybercafeMonitoringSystemTest, ClientSittingErrors) {
  TimePoint event_time = TimePoint{minutes{12 * 60}};

  ClientSatAtTableEvent unknown_client(event_time, "unknown", 1,
                                       Event::Type::kIncoming);
  unknown_client.Handle(*system);  // Should generate "ClientUnknown"

  ClientArrivedEvent arrive_event(event_time, "client1");
  arrive_event.Handle(*system);

  ClientSatAtTableEvent sit_event1(event_time, "client1", 1,
                                   Event::Type::kIncoming);
  sit_event1.Handle(*system);

  ClientArrivedEvent arrive_event2(event_time, "client2");
  arrive_event2.Handle(*system);

  ClientSatAtTableEvent sit_event2(event_time, "client2", 1,
                                   Event::Type::kIncoming);
  sit_event2.Handle(*system);  // Should generate "PlaceIsBusy"
}

TEST_F(CybercafeMonitoringSystemTest, InvalidTableOperations) {
  TimePoint event_time = TimePoint{minutes{12 * 60}};

  ClientArrivedEvent arrive_event(event_time, "client1");
  arrive_event.Handle(*system);

  EXPECT_THROW(
      ClientSatAtTableEvent(event_time, "client1", 0, Event::Type::kIncoming)
          .Handle(*system),
      std::invalid_argument);
  EXPECT_THROW(ClientSatAtTableEvent(event_time, "client1", tables_count + 1,
                                     Event::Type::kIncoming)
                   .Handle(*system),
               std::invalid_argument);
}

TEST_F(CybercafeMonitoringSystemTest, EndOfDayProcessing) {
  TimePoint event_time = TimePoint{minutes{13 * 60}};
  for (int i = 1; i <= tables_count; ++i) {
    std::string client = "client" + std::to_string(i);
    ClientArrivedEvent arrive_event(event_time, client);
    arrive_event.Handle(*system);

    ClientSatAtTableEvent sit_event(event_time, client, i,
                                    Event::Type::kIncoming);
    sit_event.Handle(*system);
  }

  ClientArrivedEvent arrive_event(event_time, "waiting_client");
  arrive_event.Handle(*system);
  ClientWaitingEvent wait_event(event_time, "waiting_client");
  wait_event.Handle(*system);

  system->EndWorkDayTrigger();

  EXPECT_GT(system->GetTotalRevenue(), 0);
}

TEST_F(CybercafeMonitoringSystemTest, IsWorkingTimeCheck) {
  TimePoint before_open = TimePoint{minutes{9 * 60}};    // 09:00
  TimePoint working_time = TimePoint{minutes{15 * 60}};  // 15:00
  TimePoint after_close = TimePoint{minutes{23 * 60}};   // 23:00

  EXPECT_FALSE(system->IsWorking(before_open));
  EXPECT_TRUE(system->IsWorking(working_time));
  EXPECT_FALSE(system->IsWorking(after_close));
}

TEST_F(CybercafeMonitoringSystemTest, TableAvailabilityCheck) {
  TimePoint event_time = TimePoint{minutes{12 * 60}};

  for (int i = 1; i <= tables_count; ++i) {
    EXPECT_TRUE(system->IsTableFree(i));
  }

  ClientArrivedEvent arrive_event(event_time, "client1");
  arrive_event.Handle(*system);
  ClientSatAtTableEvent sit_event(event_time, "client1", 1,
                                  Event::Type::kIncoming);
  sit_event.Handle(*system);

  EXPECT_FALSE(system->IsTableFree(1));
  for (int i = 2; i <= tables_count; ++i) {
    EXPECT_TRUE(system->IsTableFree(i));
  }
}

TEST_F(CybercafeMonitoringSystemTest, EdgeCaseTimeChecks) {
  ClientArrivedEvent open_time_event(opening_time, "client1");
  EXPECT_NO_THROW(open_time_event.Handle(*system));

  TimePoint before_close = closing_time - minutes{1};
  ClientArrivedEvent before_close_event(before_close, "client2");
  EXPECT_NO_THROW(before_close_event.Handle(*system));

  ClientArrivedEvent close_time_event(closing_time, "client3");
  close_time_event.Handle(*system);
}

TEST_F(CybercafeMonitoringSystemTest, FullWaitingQueue) {
  TimePoint event_time = TimePoint{minutes{12 * 60}};

  for (int i = 1; i <= tables_count; ++i) {
    std::string client = "client" + std::to_string(i);
    ClientArrivedEvent arrive_event(event_time, client);
    arrive_event.Handle(*system);

    ClientSatAtTableEvent sit_event(event_time, client, i,
                                    Event::Type::kIncoming);
    sit_event.Handle(*system);
  }

  for (int i = 1; i <= tables_count; ++i) {
    std::string client = "waiting_" + std::to_string(i);
    ClientArrivedEvent arrive_event(event_time, client);
    arrive_event.Handle(*system);

    ClientWaitingEvent wait_event(event_time, client);
    wait_event.Handle(*system);
  }

  std::string extra_client = "extra_client";
  ClientArrivedEvent arrive_event(event_time, extra_client);
  arrive_event.Handle(*system);

  ClientWaitingEvent wait_event(event_time, extra_client);
  wait_event.Handle(*system);
}

TEST_F(CybercafeMonitoringSystemTest, ClientChangesTable) {
  TimePoint event_time = TimePoint{minutes{12 * 60}};

  ClientArrivedEvent arrive_event(event_time, "client1");
  arrive_event.Handle(*system);

  ClientSatAtTableEvent sit_event1(event_time, "client1", 1,
                                   Event::Type::kIncoming);
  sit_event1.Handle(*system);
  EXPECT_FALSE(system->IsTableFree(1));

  ClientSatAtTableEvent sit_event2(event_time, "client1", 2,
                                   Event::Type::kIncoming);
  sit_event2.Handle(*system);
  EXPECT_TRUE(system->IsTableFree(1));
  EXPECT_FALSE(system->IsTableFree(2));
}

TEST_F(CybercafeMonitoringSystemTest, ErrorEventHandling) {
  TimePoint event_time = TimePoint{minutes{12 * 60}};

  ErrorEvent error_event(event_time, "Test error message");
  EXPECT_NO_THROW(error_event.Handle(*system));
}

TEST_F(CybercafeMonitoringSystemTest, RevenueAndTimeCalculation) {
  TimePoint arrive_time = TimePoint{minutes{10 * 60}};
  TimePoint depart_time = TimePoint{minutes{10 * 60 + 59}};

  ClientArrivedEvent arrive_event(arrive_time, "client1");
  arrive_event.Handle(*system);

  ClientSatAtTableEvent sit_event(arrive_time, "client1", 1,
                                  Event::Type::kIncoming);
  sit_event.Handle(*system);

  ClientLeftEvent leave_event(depart_time, "client1", Event::Type::kIncoming);
  leave_event.Handle(*system);

  EXPECT_EQ(system->GetTotalRevenue(), hourly_rate);

  TimePoint long_depart_time = TimePoint{minutes{10 * 60 + 121}};
  ClientArrivedEvent arrive_event2(arrive_time, "client2");
  arrive_event2.Handle(*system);

  ClientSatAtTableEvent sit_event2(arrive_time, "client2", 1,
                                   Event::Type::kIncoming);
  sit_event2.Handle(*system);

  ClientLeftEvent leave_event2(long_depart_time, "client2",
                               Event::Type::kIncoming);
  leave_event2.Handle(*system);

  EXPECT_EQ(system->GetTotalRevenue(), hourly_rate * 4);
}

TEST_F(CybercafeMonitoringSystemTest, ClientLeavesWithoutSitting) {
  TimePoint arrive_time = TimePoint{minutes{12 * 60}};
  TimePoint depart_time = TimePoint{minutes{12 * 60 + 30}};

  ClientArrivedEvent arrive_event(arrive_time, "client1");
  arrive_event.Handle(*system);

  ClientLeftEvent leave_event(depart_time, "client1", Event::Type::kIncoming);
  EXPECT_NO_THROW(leave_event.Handle(*system));

  EXPECT_EQ(system->GetTotalRevenue(), 0);
}

TEST_F(CybercafeMonitoringSystemTest, WaitingClientTakesFreedTable) {
  TimePoint event_time = TimePoint{minutes{12 * 60}};

  for (int i = 1; i <= tables_count; ++i) {
    std::string client = "client" + std::to_string(i);
    ClientArrivedEvent arrive_event(event_time, client);
    arrive_event.Handle(*system);

    ClientSatAtTableEvent sit_event(event_time, client, i,
                                    Event::Type::kIncoming);
    sit_event.Handle(*system);
  }

  std::string waiting_client = "waiting_client";
  ClientArrivedEvent arrive_event(event_time, waiting_client);
  arrive_event.Handle(*system);

  ClientWaitingEvent wait_event(event_time, waiting_client);
  wait_event.Handle(*system);

  ClientLeftEvent leave_event(event_time, "client1", Event::Type::kIncoming);
  leave_event.Handle(*system);

  EXPECT_FALSE(system->IsTableFree(1));
}

TEST_F(CybercafeMonitoringSystemTest, TableCountBoundaries) {
  CybercafeMonitoringSystem single_table_system(opening_time, closing_time, 1,
                                                hourly_rate);
  single_table_system.StartWorkDayTrigger();

  TimePoint event_time = TimePoint{minutes{12 * 60}};
  ClientArrivedEvent arrive_event(event_time, "client1");
  arrive_event.Handle(single_table_system);

  ClientSatAtTableEvent sit_event(event_time, "client1", 1,
                                  Event::Type::kIncoming);
  EXPECT_NO_THROW(sit_event.Handle(single_table_system));

  EXPECT_THROW(
      CybercafeMonitoringSystem(opening_time, closing_time, 0, hourly_rate),
      std::invalid_argument);
}

TEST_F(CybercafeMonitoringSystemTest, AllEventTypesHandling) {
  TimePoint event_time = TimePoint{minutes{12 * 60}};

  // k1 - ClientArrived
  ClientArrivedEvent arrive_event(event_time, "client1");
  EXPECT_NO_THROW(arrive_event.Handle(*system));

  // k2 - ClientSatAtTable (incoming)
  ClientSatAtTableEvent sit_event(event_time, "client1", 1,
                                  Event::Type::kIncoming);
  EXPECT_NO_THROW(sit_event.Handle(*system));

  // k3 - ClientWaiting
  for (int i = 2; i <= tables_count; ++i) {
    std::string client = "client" + std::to_string(i);
    ClientArrivedEvent ev(event_time, client);
    ev.Handle(*system);
    ClientSatAtTableEvent se(event_time, client, i, Event::Type::kIncoming);
    se.Handle(*system);
  }
  ClientWaitingEvent wait_event(event_time, "waiting_client");
  EXPECT_NO_THROW(wait_event.Handle(*system));

  // k4 - ClientLeft (incoming)
  ClientLeftEvent leave_event(event_time, "client1", Event::Type::kIncoming);
  EXPECT_NO_THROW(leave_event.Handle(*system));

  // k11 - ClientLeft (outgoing)
  ClientLeftEvent leave_event2(event_time, "client2", Event::Type::kOutgoing);
  EXPECT_NO_THROW(leave_event2.Handle(*system));

  // k12 - ClientSatAtTable (outgoing)
  ClientSatAtTableEvent sit_event2(event_time, "waiting_client", 1,
                                   Event::Type::kOutgoing);
  EXPECT_NO_THROW(sit_event2.Handle(*system));

  // k13 - ErrorEvent
  ErrorEvent error_event(event_time, "Test error");
  EXPECT_NO_THROW(error_event.Handle(*system));
}

TEST_F(CybercafeMonitoringSystemTest, NonStandardValidClientNames) {
  TimePoint event_time = TimePoint{minutes{12 * 60}};

  const std::string valid_names[] = {"a",
                                     "z",
                                     "0",
                                     "9",
                                     "_",
                                     "-",
                                     "a1",
                                     "z9",
                                     "a_",
                                     "z-",
                                     "a1_b2-c3",
                                     "x_9-y_8",
                                     "abcdefghijklmnopqrstuvwxyz0123456789_-"};

  for (const auto& name : valid_names) {
    ClientArrivedEvent arrive_event(event_time, name);
    EXPECT_NO_THROW(arrive_event.Handle(*system));

    ClientLeftEvent leave_event(event_time, name, Event::Type::kIncoming);
    EXPECT_NO_THROW(leave_event.Handle(*system));
  }
}

TEST_F(CybercafeMonitoringSystemTest, ZeroTimeHandling) {
  TimePoint zero_time = TimePoint{minutes{0}};

  ClientArrivedEvent arrive_event(zero_time, "client1");
  arrive_event.Handle(*system);  // Should generate "NotOpenYet"

  EXPECT_FALSE(system->IsWorking(zero_time));
}

TEST_F(CybercafeMonitoringSystemTest, ZeroTimeDifferenceSequence) {
  TimePoint event_time = TimePoint{minutes{12 * 60}};

  ClientArrivedEvent arrive1(event_time, "client1");
  arrive1.Handle(*system);
  ClientSatAtTableEvent sit1(event_time, "client1", 1, Event::Type::kIncoming);
  sit1.Handle(*system);

  ClientArrivedEvent arrive2(event_time, "client2");
  arrive2.Handle(*system);
  ClientSatAtTableEvent sit2(event_time, "client2", 2, Event::Type::kIncoming);
  sit2.Handle(*system);

  ClientLeftEvent leave1(event_time, "client1", Event::Type::kIncoming);
  leave1.Handle(*system);

  EXPECT_TRUE(system->IsTableFree(1));
  EXPECT_FALSE(system->IsTableFree(2));
}

TEST_F(CybercafeMonitoringSystemTest, MaximumStayDuration) {
  TimePoint arrive_time = opening_time;
  TimePoint depart_time = closing_time - minutes{1};

  ClientArrivedEvent arrive_event(arrive_time, "client1");
  arrive_event.Handle(*system);

  ClientSatAtTableEvent sit_event(arrive_time, "client1", 1,
                                  Event::Type::kIncoming);
  sit_event.Handle(*system);

  ClientLeftEvent leave_event(depart_time, "client1", Event::Type::kIncoming);
  leave_event.Handle(*system);

  int expected_hours = 12;
  EXPECT_EQ(system->GetTotalRevenue(), hourly_rate * expected_hours);
}

TEST_F(CybercafeMonitoringSystemTest, PostClosingEvents) {
  TimePoint after_close = closing_time + minutes{60};

  ClientArrivedEvent arrive_event(after_close, "client1");
  arrive_event.Handle(*system);  // Should generate  "NotOpenYet"

  ClientSatAtTableEvent sit_event(after_close, "client1", 1,
                                  Event::Type::kIncoming);
  sit_event.Handle(*system);  // Should generate "ClientUnknown"

  EXPECT_FALSE(system->IsWorking(after_close));
}

TEST_F(CybercafeMonitoringSystemTest, LeavingNonExistentClient) {
  TimePoint event_time = TimePoint{minutes{12 * 60}};

  ClientLeftEvent leave_event(event_time, "ghost_client",
                              Event::Type::kIncoming);
  leave_event.Handle(*system);  // Should generate "ClientUnknown"
}

}  // namespace
