#pragma once

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>

#include "api/clock.h"
#include "config/logging.h"

namespace rockvn::user::api {

// Emits exactly one structured log line per request and owns the request-ID
// contract: an incoming X-Request-Id is honored, a UUID is generated
// otherwise, and the ID is echoed on the response and stamped on the line.
// Wired as Drogon pre-routing / post-handling observers by the composition
// root — framework hooks live here, in api/, and nowhere else.
class RequestLogger {
 public:
  RequestLogger(logging::StructuredLogger& log, const Clock& clock);

  void on_request_start(const drogon::HttpRequestPtr& request) const;
  void on_request_complete(const drogon::HttpRequestPtr& request,
                           const drogon::HttpResponsePtr& response) const;

 private:
  logging::StructuredLogger& log_;
  const Clock& clock_;
};

}  // namespace rockvn::user::api
