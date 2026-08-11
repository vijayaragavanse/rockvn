#pragma once

#include <chrono>
#include <string>

namespace rockvn::user::domain {

// The aggregate this service owns. email is immutable after creation —
// identity semantics (verification, login) arrive with auth in M4, and
// allowing identity edits before those rules exist would be designing
// blind. name is the mutable profile field.
struct User {
  std::string id;
  std::string email;
  std::string name;
  std::chrono::system_clock::time_point created_at;

  friend bool operator==(const User&, const User&) = default;
};

}  // namespace rockvn::user::domain
