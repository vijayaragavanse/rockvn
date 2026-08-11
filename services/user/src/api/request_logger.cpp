#include "api/request_logger.h"

#include <drogon/utils/Utilities.h>
#include <json/json.h>

#include <chrono>
#include <string>

namespace rockvn::user::api {
namespace {

constexpr const char* kStartAttribute = "rockvn.request_start";
constexpr const char* kRequestIdAttribute = "rockvn.request_id";

}  // namespace

RequestLogger::RequestLogger(logging::StructuredLogger& log, const Clock& clock)
    : log_(log), clock_(clock) {}

void RequestLogger::on_request_start(const drogon::HttpRequestPtr& request) const {
  request->attributes()->insert(kStartAttribute, clock_.steady_now());

  std::string request_id = request->getHeader("x-request-id");
  if (request_id.empty()) {
    request_id = drogon::utils::getUuid();
  }
  request->attributes()->insert(kRequestIdAttribute, request_id);
}

void RequestLogger::on_request_complete(const drogon::HttpRequestPtr& request,
                                        const drogon::HttpResponsePtr& response) const {
  using DoubleMillis = std::chrono::duration<double, std::milli>;

  const auto attributes = request->attributes();

  double duration_ms = 0.0;
  if (attributes->find(kStartAttribute)) {
    const auto start = attributes->get<std::chrono::steady_clock::time_point>(kStartAttribute);
    duration_ms = DoubleMillis(clock_.steady_now() - start).count();
  }

  std::string request_id;
  if (attributes->find(kRequestIdAttribute)) {
    request_id = attributes->get<std::string>(kRequestIdAttribute);
  }
  response->addHeader("X-Request-Id", request_id);

  Json::Value fields;
  fields["event"] = "http_request";
  fields["method"] = request->methodString();
  fields["path"] = request->path();
  fields["status"] = static_cast<int>(response->statusCode());
  fields["duration_ms"] = duration_ms;
  fields["request_id"] = request_id;
  log_.info(std::move(fields));
}

}  // namespace rockvn::user::api
