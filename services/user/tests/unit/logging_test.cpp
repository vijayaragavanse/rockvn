#include "config/logging.h"

#include <gtest/gtest.h>
#include <spdlog/sinks/ostream_sink.h>

#include <chrono>
#include <memory>
#include <sstream>
#include <string>

#include "config/config.h"

namespace {

using rockvn::user::config::Config;
using rockvn::user::config::LogFormat;
using rockvn::user::logging::format_iso8601_utc;
using rockvn::user::logging::StructuredLogger;

Config test_config(LogFormat format) {
  return Config{
      .http_host = "127.0.0.1",
      .http_port = 8080,
      .log_level = "info",
      .log_format = format,
      .io_threads = 1,
  };
}

Json::Value parse(const std::string& line) {
  Json::Value value;
  Json::CharReaderBuilder builder;
  std::string parse_errors;
  std::istringstream stream(line);
  const bool ok = Json::parseFromStream(builder, stream, &value, &parse_errors);
  EXPECT_TRUE(ok) << "not valid JSON: " << line << " (" << parse_errors << ")";
  return value;
}

TEST(FormatIso8601Utc, FormatsKnownInstantWithMilliseconds) {
  using namespace std::chrono_literals;
  const auto instant =
      std::chrono::sys_days{std::chrono::year{2026} / 8 / 11} + 9h + 15min + 42s + 123ms;
  EXPECT_EQ(format_iso8601_utc(instant), "2026-08-11T09:15:42.123Z");
}

TEST(StructuredLogger, JsonFormatProducesOneParseableLineWithStandardFields) {
  auto stream = std::make_shared<std::ostringstream>();
  auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(*stream);
  StructuredLogger logger("user-service", test_config(LogFormat::kJson), sink);

  Json::Value fields;
  fields["event"] = "unit_test";
  fields["answer"] = 42;
  logger.info(fields);

  const auto line = stream->str();
  const auto parsed = parse(line);
  EXPECT_EQ(parsed["event"].asString(), "unit_test");
  EXPECT_EQ(parsed["answer"].asInt(), 42);
  EXPECT_EQ(parsed["level"].asString(), "info");
  EXPECT_EQ(parsed["service"].asString(), "user-service");
  EXPECT_FALSE(parsed["ts"].asString().empty());
}

TEST(StructuredLogger, JsonFormatEscapesHostileValues) {
  auto stream = std::make_shared<std::ostringstream>();
  auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(*stream);
  StructuredLogger logger("user-service", test_config(LogFormat::kJson), sink);

  Json::Value fields;
  fields["event"] = "unit_test";
  fields["path"] = "/users/\"quoted\"\nnewline";
  logger.info(fields);

  const auto parsed = parse(stream->str());
  EXPECT_EQ(parsed["path"].asString(), "/users/\"quoted\"\nnewline");
}

TEST(StructuredLogger, LevelBelowThresholdIsSuppressed) {
  auto stream = std::make_shared<std::ostringstream>();
  auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(*stream);
  StructuredLogger logger("user-service", test_config(LogFormat::kJson), sink);

  Json::Value fields;
  fields["event"] = "should_not_appear";
  logger.log(spdlog::level::debug, fields);

  EXPECT_TRUE(stream->str().empty());
}

TEST(StructuredLogger, TextFormatContainsServiceAndFields) {
  auto stream = std::make_shared<std::ostringstream>();
  auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(*stream);
  StructuredLogger logger("user-service", test_config(LogFormat::kText), sink);

  Json::Value fields;
  fields["event"] = "unit_test";
  logger.info(fields);

  const auto line = stream->str();
  EXPECT_NE(line.find("user-service"), std::string::npos);
  EXPECT_NE(line.find("event=unit_test"), std::string::npos);
}

}  // namespace
