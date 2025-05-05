// All Rights Reserved
//
// Copyright (c) 2025, github.com/BIBlical33
//
// Revenue correct calculating test
//
// This software may not be modified without the explicit permission of the
// copyright holder. For permission requests, please contact:
// mag1str.kram@gmail.com

#include <gtest/gtest.h>

#include <chrono>

#include "include/cybercafe_monitoring_system.h"

namespace {

using cybercafe_monitoring_system::TimePoint;
using CybercafeMonitoringSystem =
    cybercafe_monitoring_system::CybercafeMonitoringSystem;
using Event = CybercafeMonitoringSystem::Event;

class RevenueTest : public ::testing::Test {
 protected:
  TimePoint MakeTime(int hour, int minute) {
    return std::chrono::time_point<std::chrono::system_clock,
                                   std::chrono::minutes>(
        std::chrono::hours(hour) + std::chrono::minutes(minute));
  }

  void SetUp() override {
    opening = MakeTime(10, 0);
    closing = MakeTime(22, 0);
    system =
        std::make_unique<CybercafeMonitoringSystem>(opening, closing, 2, 100);
    system->StartWorkDayTrigger();
  }

  void TearDown() override { system.reset(); }

  TimePoint opening;
  TimePoint closing;
  std::unique_ptr<CybercafeMonitoringSystem> system;
};

TEST_F(RevenueTest, SingleClientOneHourExactly) {
  CybercafeMonitoringSystem::ClientArrivedEvent(MakeTime(10, 0), "client1")
      .Handle(*system);
  CybercafeMonitoringSystem::ClientSatAtTableEvent(MakeTime(10, 0), "client1",
                                                   1, Event::Type::kIncoming)
      .Handle(*system);
  CybercafeMonitoringSystem::ClientLeftEvent(MakeTime(11, 0), "client1",
                                             Event::Type::kIncoming)
      .Handle(*system);

  EXPECT_EQ(system->GetTotalRevenue(), 100);
}

TEST_F(RevenueTest, SingleClientFewMinutes_StillPaysHour) {
  CybercafeMonitoringSystem::ClientArrivedEvent(MakeTime(10, 0), "client1")
      .Handle(*system);
  CybercafeMonitoringSystem::ClientSatAtTableEvent(MakeTime(10, 0), "client1",
                                                   1, Event::Type::kIncoming)
      .Handle(*system);
  CybercafeMonitoringSystem::ClientLeftEvent(MakeTime(10, 5), "client1",
                                             Event::Type::kIncoming)
      .Handle(*system);

  EXPECT_EQ(system->GetTotalRevenue(), 100);
}

TEST_F(RevenueTest, SingleClientOneHourTwentyMinutes_PaysTwoHours) {
  CybercafeMonitoringSystem::ClientArrivedEvent(MakeTime(10, 0), "client1")
      .Handle(*system);
  CybercafeMonitoringSystem::ClientSatAtTableEvent(MakeTime(10, 0), "client1",
                                                   1, Event::Type::kIncoming)
      .Handle(*system);
  CybercafeMonitoringSystem::ClientLeftEvent(MakeTime(11, 20), "client1",
                                             Event::Type::kIncoming)
      .Handle(*system);

  EXPECT_EQ(system->GetTotalRevenue(), 200);
}

TEST_F(RevenueTest, MultipleClientsDifferentTimes) {
  CybercafeMonitoringSystem::ClientArrivedEvent(MakeTime(10, 0), "client1")
      .Handle(*system);
  CybercafeMonitoringSystem::ClientSatAtTableEvent(MakeTime(10, 0), "client1",
                                                   1, Event::Type::kIncoming)
      .Handle(*system);
  CybercafeMonitoringSystem::ClientLeftEvent(MakeTime(10, 10), "client1",
                                             Event::Type::kIncoming)
      .Handle(*system);

  CybercafeMonitoringSystem::ClientArrivedEvent(MakeTime(10, 15), "client2")
      .Handle(*system);
  CybercafeMonitoringSystem::ClientSatAtTableEvent(MakeTime(10, 15), "client2",
                                                   1, Event::Type::kIncoming)
      .Handle(*system);
  CybercafeMonitoringSystem::ClientLeftEvent(MakeTime(11, 45), "client2",
                                             Event::Type::kIncoming)
      .Handle(*system);

  EXPECT_EQ(system->GetTotalRevenue(), 300);
}

TEST_F(RevenueTest, ClientsSwitchingTablesRevenueAccumulates) {
  CybercafeMonitoringSystem::ClientArrivedEvent(MakeTime(10, 0), "client1")
      .Handle(*system);
  CybercafeMonitoringSystem::ClientSatAtTableEvent(MakeTime(10, 0), "client1",
                                                   1, Event::Type::kIncoming)
      .Handle(*system);
  CybercafeMonitoringSystem::ClientSatAtTableEvent(MakeTime(10, 30), "client1",
                                                   2, Event::Type::kIncoming)
      .Handle(*system);
  CybercafeMonitoringSystem::ClientLeftEvent(MakeTime(11, 15), "client1",
                                             Event::Type::kIncoming)
      .Handle(*system);

  system->EndWorkDayTrigger();

  EXPECT_EQ(system->GetTotalRevenue(), 200);
}

}  // namespace
