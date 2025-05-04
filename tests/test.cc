// All Rights Reserved
//
// Copyright (c) 2025, github.com/BIBlical33
//
// Getting and processing input data
//
// This software may not be modified without the explicit permission of the
// copyright holder. For permission requests, please contact:
// mag1str.kram@gmail.com

#include <gtest/gtest.h>

#include <chrono>

#include "include/cybercafe_monitoring_system.h"

namespace {

using cybercafe_monitoring_system::TimePoint;
using std::chrono::minutes;
using CybercafeMonitoringSystem =
    cybercafe_monitoring_system::CybercafeMonitoringSystem;

class CybercafeTest : public ::testing::Test {
 protected:
  void SetUp() override {
    TimePoint opening(minutes(540));   // 09:00
    TimePoint closing(minutes(1140));  // 19:00
    cafe = make_unique<CybercafeMonitoringSystem>(opening, closing, 3, 10);
  }

  std::unique_ptr<CybercafeMonitoringSystem> cafe;
};

}  // namespace
