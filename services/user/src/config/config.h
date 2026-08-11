#pragma once

#include <cstdint>
#include <string>
#include <tl/expected.hpp>
#include <vector>

namespace rockvn::user::config {

enum class LogFormat { kText, kJson };

// Immutable process configuration. Read once at startup by the composition
// root; no component reads the environment after load_from_env returns.
struct Config {
  std::string http_host;
  std::uint16_t http_port;
  std::string log_level;
  LogFormat log_format;
  int io_threads;
};

struct ConfigError {
  std::string variable;
  std::string message;
};

// Reads USER_SERVICE_* variables from the process environment, applying
// defaults for absent variables. Returns every violated rule, not just the
// first, so operators fix configuration once instead of iteratively.
tl::expected<Config, std::vector<ConfigError>> load_from_env();

}  // namespace rockvn::user::config
