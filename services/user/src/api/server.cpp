#include "api/server.h"

#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>

#include <cstddef>
#include <functional>

#include "api/problem_json.h"

namespace rockvn::user::api {

void configure_app(const config::Config& cfg, HealthHandler& health,
                   RequestLogger& request_logger) {
  // The framework's own logging competes with our structured lines on
  // stdout; anything below warn is noise once our request logging exists.
  trantor::Logger::setLogLevel(trantor::Logger::kWarn);

  auto& app = drogon::app();
  app.setThreadNum(static_cast<std::size_t>(cfg.io_threads));
  app.addListener(cfg.http_host, cfg.http_port);

  app.registerHandler("/health",
                      [&health](const drogon::HttpRequestPtr& request,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
                        callback(health.handle(request));
                      },
                      {drogon::Get});

  app.registerPreRoutingAdvice([&request_logger](const drogon::HttpRequestPtr& request) {
    request_logger.on_request_start(request);
  });
  app.registerPostHandlingAdvice([&request_logger](const drogon::HttpRequestPtr& request,
                                                   const drogon::HttpResponsePtr& response) {
    request_logger.on_request_complete(request, response);
  });

  app.setCustom404Page(make_problem_response(drogon::k404NotFound, "Not Found",
                                             "The requested resource does not exist."));
}

}  // namespace rockvn::user::api
