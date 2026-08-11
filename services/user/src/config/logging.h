#pragma once

#include <json/json.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <memory>
#include <string>

#include "config/config.h"

namespace rockvn::user::logging {

// UTC timestamp with millisecond precision, e.g. "2026-08-11T09:15:42.123Z".
std::string format_iso8601_utc(std::chrono::system_clock::time_point time_point);

// Structured logger owned by the composition root and passed by reference —
// deliberately not spdlog's global registry, so tests inject a capture sink
// through the constructor instead of swapping process-wide state.
//
// Field composition goes through jsoncpp so values are always correctly
// escaped; log lines are machine-parseable by construction, never by luck.
class StructuredLogger {
 public:
  // A null sink selects stdout — the only sink production uses (12-factor:
  // the container runtime owns log routing, not the process).
  StructuredLogger(std::string service_name, const config::Config& cfg,
                   spdlog::sink_ptr sink = nullptr);

  void log(spdlog::level::level_enum level, Json::Value fields);
  void info(Json::Value fields) { log(spdlog::level::info, std::move(fields)); }
  void warn(Json::Value fields) { log(spdlog::level::warn, std::move(fields)); }
  void error(Json::Value fields) { log(spdlog::level::err, std::move(fields)); }

 private:
  std::shared_ptr<spdlog::logger> logger_;
  config::LogFormat format_;
  std::string service_name_;
};

}  // namespace rockvn::user::logging
