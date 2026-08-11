#include "api/problem_json.h"

#include <json/json.h>

namespace rockvn::user::api {

drogon::HttpResponsePtr make_problem_response(drogon::HttpStatusCode status,
                                              const std::string& title, const std::string& detail,
                                              const std::string& instance) {
  Json::Value body;
  body["type"] = "about:blank";
  body["title"] = title;
  body["status"] = static_cast<int>(status);
  body["detail"] = detail;
  if (!instance.empty()) {
    body["instance"] = instance;
  }

  Json::StreamWriterBuilder compact;
  compact["indentation"] = "";

  auto response = drogon::HttpResponse::newHttpResponse();
  response->setStatusCode(status);
  response->setBody(Json::writeString(compact, body));
  response->setContentTypeString("application/problem+json");
  return response;
}

}  // namespace rockvn::user::api
