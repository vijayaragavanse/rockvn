#pragma once

#include <drogon/HttpResponse.h>

#include <string>

namespace rockvn::user::api {

// RFC 9457 problem+json error envelope — the wire contract for every error
// response this service produces. Fixed in M1 so M2's business endpoints
// inherit a contract instead of inventing one.
drogon::HttpResponsePtr make_problem_response(drogon::HttpStatusCode status,
                                              const std::string& title, const std::string& detail,
                                              const std::string& instance = {});

}  // namespace rockvn::user::api
