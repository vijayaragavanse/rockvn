#include "config/config.h"

#include <gtest/gtest.h>

#include <cstdlib>
#include <string>

namespace {

using rockvn::user::config::load_from_env;
using rockvn::user::config::LogFormat;

void set_env(const char* name, const char* value) {
#ifdef _WIN32
  _putenv_s(name, value);
#else
  setenv(name, value, 1);
#endif
}

void unset_env(const char* name) {
#ifdef _WIN32
  _putenv_s(name, "");
#else
  unsetenv(name);
#endif
}

class ConfigTest : public ::testing::Test {
 protected:
  void SetUp() override { clear_all(); }
  void TearDown() override { clear_all(); }

  static void clear_all() {
    for (const auto* name :
         {"USER_SERVICE_HTTP_HOST", "USER_SERVICE_HTTP_PORT", "USER_SERVICE_LOG_LEVEL",
          "USER_SERVICE_LOG_FORMAT", "USER_SERVICE_IO_THREADS"}) {
      unset_env(name);
    }
  }
};

TEST_F(ConfigTest, DefaultsAreValidWithEmptyEnvironment) {
  const auto result = load_from_env();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->http_host, "0.0.0.0");
  EXPECT_EQ(result->http_port, 8080);
  EXPECT_EQ(result->log_level, "info");
  EXPECT_EQ(result->log_format, LogFormat::kText);
  EXPECT_EQ(result->io_threads, 1);
}

TEST_F(ConfigTest, ValidOverridesAreApplied) {
  set_env("USER_SERVICE_HTTP_HOST", "127.0.0.1");
  set_env("USER_SERVICE_HTTP_PORT", "9090");
  set_env("USER_SERVICE_LOG_LEVEL", "debug");
  set_env("USER_SERVICE_LOG_FORMAT", "json");
  set_env("USER_SERVICE_IO_THREADS", "4");

  const auto result = load_from_env();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->http_host, "127.0.0.1");
  EXPECT_EQ(result->http_port, 9090);
  EXPECT_EQ(result->log_level, "debug");
  EXPECT_EQ(result->log_format, LogFormat::kJson);
  EXPECT_EQ(result->io_threads, 4);
}

TEST_F(ConfigTest, PortOutOfRangeIsRejectedAndNamesTheVariable) {
  set_env("USER_SERVICE_HTTP_PORT", "99999");
  const auto result = load_from_env();
  ASSERT_FALSE(result.has_value());
  ASSERT_EQ(result.error().size(), 1);
  EXPECT_EQ(result.error()[0].variable, "USER_SERVICE_HTTP_PORT");
}

TEST_F(ConfigTest, NonNumericPortIsRejected) {
  set_env("USER_SERVICE_HTTP_PORT", "http");
  EXPECT_FALSE(load_from_env().has_value());
}

TEST_F(ConfigTest, UnknownLogLevelIsRejected) {
  set_env("USER_SERVICE_LOG_LEVEL", "loud");
  EXPECT_FALSE(load_from_env().has_value());
}

TEST_F(ConfigTest, UnknownLogFormatIsRejected) {
  set_env("USER_SERVICE_LOG_FORMAT", "xml");
  EXPECT_FALSE(load_from_env().has_value());
}

TEST_F(ConfigTest, IoThreadsBoundsAreEnforced) {
  set_env("USER_SERVICE_IO_THREADS", "0");
  EXPECT_FALSE(load_from_env().has_value());
  set_env("USER_SERVICE_IO_THREADS", "64");
  EXPECT_TRUE(load_from_env().has_value());
  set_env("USER_SERVICE_IO_THREADS", "65");
  EXPECT_FALSE(load_from_env().has_value());
}

// The acceptance criterion from the M1 design: every violated rule is
// reported in one failure, so operators fix configuration once.
TEST_F(ConfigTest, AllViolationsAreReportedTogether) {
  set_env("USER_SERVICE_HTTP_PORT", "abc");
  set_env("USER_SERVICE_LOG_LEVEL", "nope");
  const auto result = load_from_env();
  ASSERT_FALSE(result.has_value());
  ASSERT_EQ(result.error().size(), 2);
  EXPECT_EQ(result.error()[0].variable, "USER_SERVICE_HTTP_PORT");
  EXPECT_EQ(result.error()[1].variable, "USER_SERVICE_LOG_LEVEL");
}

}  // namespace
