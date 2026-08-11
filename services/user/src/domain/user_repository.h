#pragma once

#include <optional>
#include <string>
#include <tl/expected.hpp>
#include <vector>

#include "domain/errors.h"
#include "domain/user.h"

namespace rockvn::user::domain {

// Storage seam. Email uniqueness is the repository's invariant, not the
// service's: checking before inserting is a TOCTOU race, so insert itself
// reports kConflict — the in-memory implementation enforces it under a
// lock, the M3 PostgreSQL implementation via a unique index. The contract
// test suite (tests/contract/) pins these semantics for every
// implementation.
class UserRepository {
 public:
  virtual ~UserRepository() = default;

  virtual tl::expected<void, Error> insert(const User& user) = 0;
  virtual std::optional<User> find_by_id(const std::string& id) const = 0;
  // Ordered by created_at, then id — deterministic output is part of the
  // contract until pagination (M3) replaces it.
  virtual std::vector<User> find_all() const = 0;
  virtual tl::expected<void, Error> update(const User& user) = 0;
  virtual tl::expected<void, Error> remove(const std::string& id) = 0;
};

}  // namespace rockvn::user::domain
