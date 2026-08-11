#pragma once

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>

#include "domain/errors.h"

namespace rockvn::user::api {

// The one table mapping domain errors to the wire: Validation→400,
// NotFound→404, Conflict→409. Bodies are RFC 9457 problem+json with the
// request ID as `instance` and, for validation errors, a `field`
// extension member naming the rejected input.
drogon::HttpResponsePtr to_error_response(const domain::Error& error,
                                          const drogon::HttpRequestPtr& request);

}  // namespace rockvn::user::api
