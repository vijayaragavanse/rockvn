#include "api/user_handlers.h"

#include <json/json.h>

#include <optional>
#include <utility>

#include "api/error_mapping.h"
#include "api/problem_json.h"
#include "config/logging.h"

namespace rockvn::user::api {
namespace {

Json::Value to_json(const domain::User& user) {
  Json::Value body;
  body["id"] = user.id;
  body["email"] = user.email;
  body["name"] = user.name;
  body["created_at"] = logging::format_iso8601_utc(user.created_at);
  return body;
}

// Wire-shape validation happens at the boundary; the domain never sees
// unparsed input. Returns the string field or nullopt if absent/mistyped.
std::optional<std::string> string_field(const std::shared_ptr<Json::Value>& body,
                                        const char* field) {
  if (!body || !body->isObject() || !(*body)[field].isString()) {
    return std::nullopt;
  }
  return (*body)[field].asString();
}

drogon::HttpResponsePtr bad_request(const drogon::HttpRequestPtr& request, const char* detail) {
  return to_error_response({domain::ErrorKind::kValidation, detail, "body"}, request);
}

}  // namespace

UserHandlers::UserHandlers(domain::UserService& users) : users_(users) {}

drogon::HttpResponsePtr UserHandlers::create(const drogon::HttpRequestPtr& request) const {
  const auto body = request->jsonObject();
  const auto email = string_field(body, "email");
  const auto name = string_field(body, "name");
  if (!email || !name) {
    return bad_request(request, "body must be JSON with string fields 'email' and 'name'");
  }

  auto result = users_.create(*email, *name);
  if (!result) {
    return to_error_response(result.error(), request);
  }

  auto response = drogon::HttpResponse::newHttpJsonResponse(to_json(*result));
  response->setStatusCode(drogon::k201Created);
  response->addHeader("Location", "/users/" + result->id);
  return response;
}

drogon::HttpResponsePtr UserHandlers::get(const drogon::HttpRequestPtr& request,
                                          const std::string& id) const {
  auto result = users_.get(id);
  if (!result) {
    return to_error_response(result.error(), request);
  }
  return drogon::HttpResponse::newHttpJsonResponse(to_json(*result));
}

drogon::HttpResponsePtr UserHandlers::list(const drogon::HttpRequestPtr& /*request*/) const {
  Json::Value body;
  body["users"] = Json::Value(Json::arrayValue);
  for (const auto& user : users_.list()) {
    body["users"].append(to_json(user));
  }
  return drogon::HttpResponse::newHttpJsonResponse(std::move(body));
}

drogon::HttpResponsePtr UserHandlers::rename(const drogon::HttpRequestPtr& request,
                                             const std::string& id) const {
  const auto name = string_field(request->jsonObject(), "name");
  if (!name) {
    return bad_request(request, "body must be JSON with string field 'name'");
  }

  auto result = users_.rename(id, *name);
  if (!result) {
    return to_error_response(result.error(), request);
  }
  return drogon::HttpResponse::newHttpJsonResponse(to_json(*result));
}

drogon::HttpResponsePtr UserHandlers::remove(const drogon::HttpRequestPtr& request,
                                             const std::string& id) const {
  const auto result = users_.remove(id);
  if (!result) {
    return to_error_response(result.error(), request);
  }
  auto response = drogon::HttpResponse::newHttpResponse();
  response->setStatusCode(drogon::k204NoContent);
  return response;
}

}  // namespace rockvn::user::api
