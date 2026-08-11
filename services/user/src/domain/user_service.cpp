#include "domain/user_service.h"

#include <algorithm>
#include <optional>
#include <utility>

namespace rockvn::user::domain {
namespace {

constexpr std::size_t kMaxEmailLength = 254;  // RFC 5321 path limit
constexpr std::size_t kMaxNameLength = 100;

// Syntactic sanity only — full RFC 5322 validation is a famous tar pit,
// and deliverability is the mail system's problem, not this service's.
std::optional<Error> validate_email(const std::string& email) {
  if (email.empty()) {
    return Error{ErrorKind::kValidation, "email must not be empty", "email"};
  }
  if (email.size() > kMaxEmailLength) {
    return Error{ErrorKind::kValidation, "email must be at most 254 characters", "email"};
  }
  const auto at = email.find('@');
  if (at == std::string::npos || at != email.rfind('@') || at == 0 || at == email.size() - 1) {
    return Error{ErrorKind::kValidation,
                 "email must contain exactly one '@' with text on both sides", "email"};
  }
  const bool has_invalid = std::any_of(email.begin(), email.end(), [](unsigned char ch) {
    return ch <= ' ';  // spaces and control characters
  });
  if (has_invalid) {
    return Error{ErrorKind::kValidation, "email must not contain spaces or control characters",
                 "email"};
  }
  return std::nullopt;
}

std::optional<Error> validate_name(const std::string& name) {
  if (name.empty()) {
    return Error{ErrorKind::kValidation, "name must not be empty", "name"};
  }
  if (name.size() > kMaxNameLength) {
    return Error{ErrorKind::kValidation, "name must be at most 100 characters", "name"};
  }
  const bool only_whitespace =
      std::all_of(name.begin(), name.end(), [](unsigned char ch) { return ch <= ' '; });
  if (only_whitespace) {
    return Error{ErrorKind::kValidation, "name must contain visible characters", "name"};
  }
  return std::nullopt;
}

}  // namespace

UserService::UserService(UserRepository& repository, const Clock& clock,
                         const IdGenerator& id_generator)
    : repository_(repository), clock_(clock), id_generator_(id_generator) {}

tl::expected<User, Error> UserService::create(const std::string& email, const std::string& name) {
  if (auto error = validate_email(email)) {
    return tl::unexpected(std::move(*error));
  }
  if (auto error = validate_name(name)) {
    return tl::unexpected(std::move(*error));
  }

  User user{
      .id = id_generator_.generate(),
      .email = email,
      .name = name,
      .created_at = clock_.system_now(),
  };
  // Uniqueness is the repository's invariant — pre-checking here would be
  // a TOCTOU race. Conflict propagates as a value.
  if (auto inserted = repository_.insert(user); !inserted) {
    return tl::unexpected(std::move(inserted.error()));
  }
  return user;
}

tl::expected<User, Error> UserService::get(const std::string& id) const {
  if (auto user = repository_.find_by_id(id)) {
    return std::move(*user);
  }
  return tl::unexpected(Error{ErrorKind::kNotFound, "user not found", ""});
}

std::vector<User> UserService::list() const { return repository_.find_all(); }

tl::expected<User, Error> UserService::rename(const std::string& id, const std::string& new_name) {
  if (auto error = validate_name(new_name)) {
    return tl::unexpected(std::move(*error));
  }
  auto user = repository_.find_by_id(id);
  if (!user) {
    return tl::unexpected(Error{ErrorKind::kNotFound, "user not found", ""});
  }
  user->name = new_name;
  if (auto updated = repository_.update(*user); !updated) {
    return tl::unexpected(std::move(updated.error()));
  }
  return std::move(*user);
}

tl::expected<void, Error> UserService::remove(const std::string& id) {
  return repository_.remove(id);
}

}  // namespace rockvn::user::domain
