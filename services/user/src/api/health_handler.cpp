#include "api/health_handler.h"

#include <utility>

#include "config/logging.h"

namespace rockvn::user::api {

HealthHandler::HealthHandler(std::string service_name, const domain::Clock& clock,
                             BuildInfo build_info)
    : service_name_(std::move(service_name)),
      clock_(clock),
      build_info_(std::move(build_info)),
      started_(clock.steady_now()) {}

Json::Value HealthHandler::status() const {
  using std::chrono::duration_cast;
  using std::chrono::seconds;

  Json::Value body;
  body["status"] = "ok";
  body["service"] = service_name_;
  body["version"] = build_info_.version + "+g" + build_info_.git_sha;
  body["uptime_seconds"] =
      static_cast<Json::Int64>(duration_cast<seconds>(clock_.steady_now() - started_).count());
  body["timestamp"] = logging::format_iso8601_utc(clock_.system_now());
  return body;
}

drogon::HttpResponsePtr HealthHandler::handle(const drogon::HttpRequestPtr& /*request*/) const {
  return drogon::HttpResponse::newHttpJsonResponse(status());
}

}  // namespace rockvn::user::api
