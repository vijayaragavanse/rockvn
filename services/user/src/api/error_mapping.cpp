#include "api/error_mapping.h"

#include <json/json.h>

#include <string>

#include "api/request_logger.h"

namespace rockvn::user::api {

drogon::HttpResponsePtr to_error_response(const domain::Error& error,
                                          const drogon::HttpRequestPtr& request) {
  drogon::HttpStatusCode status = drogon::k500InternalServerError;
  std::string title = "Internal Server Error";
  switch (error.kind) {
    case domain::ErrorKind::kValidation:
      status = drogon::k400BadRequest;
      title = "Validation Failed";
      break;
    case domain::ErrorKind::kNotFound:
      status = drogon::k404NotFound;
      title = "Not Found";
      break;
    case domain::ErrorKind::kConflict:
      status = drogon::k409Conflict;
      title = "Conflict";
      break;
  }

  Json::Value body;
  body["type"] = "about:blank";
  body["title"] = title;
  body["status"] = static_cast<int>(status);
  body["detail"] = error.message;
  const auto request_id = request_id_of(request);
  if (!request_id.empty()) {
    body["instance"] = "urn:request-id:" + request_id;
  }
  if (error.kind == domain::ErrorKind::kValidation && !error.field.empty()) {
    body["field"] = error.field;  // problem+json extension member
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
