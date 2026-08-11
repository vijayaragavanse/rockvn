#include "config/config.h"

#include <array>
#include <charconv>
#include <cstdlib>
#include <optional>
#include <string_view>

namespace rockvn::user::config {
namespace {

std::optional<std::string> env(const char* name) {
  const char* value = std::getenv(name);
  if (value == nullptr || *value == '\0') {
    return std::nullopt;
  }
  return std::string{value};
}

std::optional<int> parse_int(const std::string& text) {
  int value{};
  const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
  if (ec != std::errc{} || ptr != text.data() + text.size()) {
    return std::nullopt;
  }
  return value;
}

}  // namespace

tl::expected<Config, std::vector<ConfigError>> load_from_env() {
  std::vector<ConfigError> errors;
  Config cfg{
      .http_host = "0.0.0.0",
      .http_port = 8080,
      .log_level = "info",
      .log_format = LogFormat::kText,
      .io_threads = 1,
  };

  if (const auto host = env("USER_SERVICE_HTTP_HOST")) {
    cfg.http_host = *host;
  }

  if (const auto port = env("USER_SERVICE_HTTP_PORT")) {
    const auto parsed = parse_int(*port);
    if (parsed && *parsed >= 1 && *parsed <= 65535) {
      cfg.http_port = static_cast<std::uint16_t>(*parsed);
    } else {
      errors.push_back(
          {"USER_SERVICE_HTTP_PORT", "must be an integer in [1, 65535], got '" + *port + "'"});
    }
  }

  if (const auto level = env("USER_SERVICE_LOG_LEVEL")) {
    constexpr std::array<std::string_view, 5> kAllowed{"trace", "debug", "info", "warn", "error"};
    bool known = false;
    for (const auto candidate : kAllowed) {
      known = known || candidate == *level;
    }
    if (known) {
      cfg.log_level = *level;
    } else {
      errors.push_back({"USER_SERVICE_LOG_LEVEL",
                        "must be one of trace|debug|info|warn|error, got '" + *level + "'"});
    }
  }

  if (const auto format = env("USER_SERVICE_LOG_FORMAT")) {
    if (*format == "text") {
      cfg.log_format = LogFormat::kText;
    } else if (*format == "json") {
      cfg.log_format = LogFormat::kJson;
    } else {
      errors.push_back(
          {"USER_SERVICE_LOG_FORMAT", "must be 'text' or 'json', got '" + *format + "'"});
    }
  }

  if (const auto threads = env("USER_SERVICE_IO_THREADS")) {
    const auto parsed = parse_int(*threads);
    if (parsed && *parsed >= 1 && *parsed <= 64) {
      cfg.io_threads = *parsed;
    } else {
      errors.push_back(
          {"USER_SERVICE_IO_THREADS", "must be an integer in [1, 64], got '" + *threads + "'"});
    }
  }

  if (!errors.empty()) {
    return tl::unexpected(std::move(errors));
  }
  return cfg;
}

}  // namespace rockvn::user::config
