// All Rights Reserved
//
// Copyright (c) 2025, github.com/BIBlical33
//
// Getting and processing input data testing
//
// This software may not be modified without the explicit permission of the
// copyright holder. For permission requests, please contact:
// mag1str.kram@gmail.com

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "include/cybercafe_monitoring_system.h"
#include "include/read_input_data.h"

namespace {

class CybercafeSystemTest : public ::testing::Test {
 protected:
  void SetUp() override {
    temp_dir = std::filesystem::temp_directory_path() / "cybercafe_tests";
    std::filesystem::create_directory(temp_dir);
  }

  void TearDown() override { std::filesystem::remove_all(temp_dir); }

  void CreateTestFile(const std::string& filename, const std::string& content) const {
    std::ofstream out(temp_dir / filename);
    out << content;
    out.close();
  }

  std::string RunSystemWithInput(const std::string& filename) const {
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    std::ifstream in(temp_dir / filename);
    if (!in.is_open()) {
      throw std::runtime_error("Cannot open test file");
    }

    try {
      cybercafe_monitoring_system_test::ProcessingInputData(in);
    } catch (...) {
      std::cout.rdbuf(old);
      throw;
    }

    std::cout.rdbuf(old);
    return buffer.str();
  }

  std::filesystem::path temp_dir;
};

TEST_F(CybercafeSystemTest, ValidInput) {
  std::string input_content =
      "3\n"
      "08:00 20:00\n"
      "10\n"
      "08:15 1 client1\n"
      "08:20 2 client1 1\n"
      "09:30 4 client1\n";

  CreateTestFile("valid.txt", input_content);

  EXPECT_NO_THROW({
    std::string output = RunSystemWithInput("valid.txt");
    EXPECT_NE(output.find("08:00"), std::string::npos);
    EXPECT_NE(output.find("client1"), std::string::npos);
    EXPECT_NE(output.find("20:00"), std::string::npos);
  });
}

TEST_F(CybercafeSystemTest, InvalidTimeFormat) {
  std::string input_content =
      "3\n"
      "25:00 20:00\n"
      "10\n";

  CreateTestFile("invalid_time.txt", input_content);

  EXPECT_THROW({ RunSystemWithInput("invalid_time.txt"); }, std::runtime_error);
}

TEST_F(CybercafeSystemTest, InvalidClientName) {
  std::string input_content =
      "3\n"
      "08:00 20:00\n"
      "10\n"
      "08:15 1 Client@Invalid\n";

  CreateTestFile("invalid_client.txt", input_content);

  EXPECT_THROW(
      { RunSystemWithInput("invalid_client.txt"); }, std::runtime_error);
}

TEST_F(CybercafeSystemTest, EmptyFile) {
  std::string input_content = "";
  CreateTestFile("empty.txt", input_content);

  EXPECT_THROW({ RunSystemWithInput("empty.txt"); }, std::runtime_error);
}

TEST_F(CybercafeSystemTest, InvalidTableCount) {
  std::string input_content =
      "0\n"
      "08:00 20:00\n"
      "10\n";

  CreateTestFile("invalid_tables.txt", input_content);

  EXPECT_THROW(
      { RunSystemWithInput("invalid_tables.txt"); }, std::runtime_error);
}

}  // namespace
