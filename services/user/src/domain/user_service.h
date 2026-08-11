#pragma once

#include <string>
#include <tl/expected.hpp>
#include <vector>

#include "domain/clock.h"
#include "domain/errors.h"
#include "domain/id_generator.h"
#include "domain/user.h"
#include "domain/user_repository.h"

namespace rockvn::user::domain {

// Business logic for the User aggregate. Framework-free by construction:
// this target cannot link Drogon, so transport concerns physically cannot
// leak in. All failures a caller can act on travel as values (ADR-0007).
class UserService {
 public:
  UserService(UserRepository& repository, const Clock& clock, const IdGenerator& id_generator);

  tl::expected<User, Error> create(const std::string& email, const std::string& name);
  tl::expected<User, Error> get(const std::string& id) const;
  std::vector<User> list() const;
  tl::expected<User, Error> rename(const std::string& id, const std::string& new_name);
  tl::expected<void, Error> remove(const std::string& id);

 private:
  UserRepository& repository_;
  const Clock& clock_;
  const IdGenerator& id_generator_;
};

}  // namespace rockvn::user::domain
