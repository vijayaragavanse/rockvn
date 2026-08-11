#include "config/logging.h"

#include <spdlog/sinks/stdout_sinks.h>

#include <cstdio>
#include <ctime>
#include <utility>

namespace rockvn::user::logging {

std::string format_iso8601_utc(std::chrono::system_clock::time_point time_point) {
  using std::chrono::duration_cast;
  using std::chrono::milliseconds;

  const auto since_epoch = time_point.time_since_epoch();
  const auto millis = duration_cast<milliseconds>(since_epoch).count() % 1000;
  const std::time_t seconds = std::chrono::system_clock::to_time_t(time_point);

  std::tm utc{};
#ifdef _WIN32
  gmtime_s(&utc, &seconds);
#else
  gmtime_r(&seconds, &utc);
#endif

  char buffer[40];
  const auto written = std::strftime(buffer, sizeof buffer, "%Y-%m-%dT%H:%M:%S", &utc);
  std::snprintf(buffer + written, sizeof buffer - written, ".%03dZ", static_cast<int>(millis));
  return buffer;
}

StructuredLogger::StructuredLogger(std::string service_name, const config::Config& cfg,
                                   spdlog::sink_ptr sink)
    : format_(cfg.log_format), service_name_(std::move(service_name)) {
  if (!sink) {
    sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
  }
  logger_ = std::make_shared<spdlog::logger>(service_name_, std::move(sink));
  logger_->set_pattern("%v");  // lines are fully composed here; spdlog only delivers them
  logger_->set_level(spdlog::level::from_str(cfg.log_level));
  // Flush per record: log lines must be observable the moment they happen
  // (tests and `docker logs -f` both rely on it). Revisit if M8 measures a cost.
  logger_->flush_on(spdlog::level::trace);
}

void StructuredLogger::log(spdlog::level::level_enum level, Json::Value fields) {
  const auto level_name = spdlog::level::to_string_view(level);
  fields["ts"] = format_iso8601_utc(std::chrono::system_clock::now());
  fields["level"] = std::string(level_name.data(), level_name.size());
  fields["service"] = service_name_;

  Json::StreamWriterBuilder compact;
  compact["indentation"] = "";

  if (format_ == config::LogFormat::kJson) {
    logger_->log(level, Json::writeString(compact, fields));
    return;
  }

  std::string line =
      fields["ts"].asString() + " [" + fields["level"].asString() + "] " + service_name_;
  for (const auto& key : fields.getMemberNames()) {
    if (key == "ts" || key == "level" || key == "service") {
      continue;
    }
    const auto& value = fields[key];
    line +=
        " " + key + "=" + (value.isString() ? value.asString() : Json::writeString(compact, value));
  }
  logger_->log(level, line);
}

}  // namespace rockvn::user::logging
