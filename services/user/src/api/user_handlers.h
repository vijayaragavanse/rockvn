#pragma once

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>

#include <string>

#include "domain/user_service.h"

namespace rockvn::user::api {

// Transport adapters for the User endpoints: parse and validate the wire
// shape, delegate to the domain service, map results and errors back to
// HTTP. No business rules live here — a handler that grows an `if` about
// users (rather than about JSON) is in the wrong layer.
class UserHandlers {
 public:
  explicit UserHandlers(domain::UserService& users);

  drogon::HttpResponsePtr create(const drogon::HttpRequestPtr& request) const;
  drogon::HttpResponsePtr get(const drogon::HttpRequestPtr& request, const std::string& id) const;
  drogon::HttpResponsePtr list(const drogon::HttpRequestPtr& request) const;
  drogon::HttpResponsePtr rename(const drogon::HttpRequestPtr& request,
                                 const std::string& id) const;
  drogon::HttpResponsePtr remove(const drogon::HttpRequestPtr& request,
                                 const std::string& id) const;

 private:
  domain::UserService& users_;
};

}  // namespace rockvn::user::api
