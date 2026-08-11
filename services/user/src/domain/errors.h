#pragma once

#include <string>

namespace rockvn::user::domain {

// The domain's error vocabulary. api/ owns the mapping to HTTP statuses
// (Validation→400, NotFound→404, Conflict→409); domain code neither knows
// nor cares that HTTP exists.
enum class ErrorKind { kValidation, kNotFound, kConflict };

struct Error {
  ErrorKind kind;
  std::string message;
  std::string field;  // set for kValidation: which input was rejected

  friend bool operator==(const Error&, const Error&) = default;
};

}  // namespace rockvn::user::domain
