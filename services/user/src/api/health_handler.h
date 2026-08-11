#pragma once

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <json/json.h>

#include <chrono>
#include <string>

#include "domain/clock.h"

namespace rockvn::user::api {

struct BuildInfo {
  std::string version;
  std::string git_sha;
};

// Liveness endpoint. status() is the framework-free core — unit-testable
// with a fake clock; handle() is the thin Drogon adapter around it.
class HealthHandler {
 public:
  HealthHandler(std::string service_name, const domain::Clock& clock, BuildInfo build_info);

  Json::Value status() const;
  drogon::HttpResponsePtr handle(const drogon::HttpRequestPtr& request) const;

 private:
  std::string service_name_;
  const domain::Clock& clock_;
  BuildInfo build_info_;
  std::chrono::steady_clock::time_point started_;
};

}  // namespace rockvn::user::api
