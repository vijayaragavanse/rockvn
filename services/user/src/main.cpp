// Composition root: the only file that knows the object graph. Construct,
// wire, run — no logic. Everything constructed here outlives
// drogon::app().run() and is destroyed in reverse order afterward (RAII).

#include <drogon/drogon.h>
#include <json/json.h>

#include <cstdio>
#include <string>

#include "api/clock.h"
#include "api/health_handler.h"
#include "api/request_logger.h"
#include "api/server.h"
#include "config/build_info.h"
#include "config/config.h"
#include "config/logging.h"

int main() {
  const auto cfg_result = rockvn::user::config::load_from_env();
  if (!cfg_result) {
    // The logger is configuration-dependent, so configuration failures go
    // to stderr directly — every violation, then a non-zero exit before
    // any listener opens (fail fast).
    for (const auto& error : cfg_result.error()) {
      std::fprintf(stderr, "config error: %s: %s\n", error.variable.c_str(), error.message.c_str());
    }
    return 1;
  }
  const auto& cfg = *cfg_result;

  const std::string service_name = "user-service";
  rockvn::user::logging::StructuredLogger log(service_name, cfg);
  rockvn::user::api::SystemClock clock;
  rockvn::user::api::HealthHandler health(service_name, clock,
                                          {rockvn::user::kVersion, rockvn::user::kGitSha});
  rockvn::user::api::RequestLogger request_logger(log, clock);

  rockvn::user::api::configure_app(cfg, health, request_logger);

  Json::Value startup;
  startup["event"] = "startup";
  startup["version"] = std::string(rockvn::user::kVersion) + "+g" + rockvn::user::kGitSha;
  startup["host"] = cfg.http_host;
  startup["port"] = cfg.http_port;
  startup["io_threads"] = cfg.io_threads;
  log.info(std::move(startup));

  drogon::app().run();

  Json::Value shutdown;
  shutdown["event"] = "shutdown";
  log.info(std::move(shutdown));
  return 0;
}
