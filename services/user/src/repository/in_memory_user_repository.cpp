#include "repository/in_memory_user_repository.h"

#include <algorithm>

namespace rockvn::user::repository {

using domain::Error;
using domain::ErrorKind;
using domain::User;

tl::expected<void, Error> InMemoryUserRepository::insert(const User& user) {
  const std::lock_guard<std::mutex> lock(mutex_);
  for (const auto& [id, existing] : users_by_id_) {
    if (existing.email == user.email) {
      return tl::unexpected(Error{ErrorKind::kConflict, "email already registered", "email"});
    }
  }
  const auto [it, inserted] = users_by_id_.emplace(user.id, user);
  if (!inserted) {
    return tl::unexpected(Error{ErrorKind::kConflict, "id already exists", "id"});
  }
  return {};
}

std::optional<User> InMemoryUserRepository::find_by_id(const std::string& id) const {
  const std::lock_guard<std::mutex> lock(mutex_);
  const auto it = users_by_id_.find(id);
  if (it == users_by_id_.end()) {
    return std::nullopt;
  }
  return it->second;
}

std::vector<User> InMemoryUserRepository::find_all() const {
  std::vector<User> users;
  {
    const std::lock_guard<std::mutex> lock(mutex_);
    users.reserve(users_by_id_.size());
    for (const auto& [id, user] : users_by_id_) {
      users.push_back(user);
    }
  }
  std::sort(users.begin(), users.end(), [](const User& lhs, const User& rhs) {
    return lhs.created_at != rhs.created_at ? lhs.created_at < rhs.created_at : lhs.id < rhs.id;
  });
  return users;
}

tl::expected<void, Error> InMemoryUserRepository::update(const User& user) {
  const std::lock_guard<std::mutex> lock(mutex_);
  const auto it = users_by_id_.find(user.id);
  if (it == users_by_id_.end()) {
    return tl::unexpected(Error{ErrorKind::kNotFound, "user not found", ""});
  }
  for (const auto& [id, existing] : users_by_id_) {
    if (id != user.id && existing.email == user.email) {
      return tl::unexpected(Error{ErrorKind::kConflict, "email already registered", "email"});
    }
  }
  it->second = user;
  return {};
}

tl::expected<void, Error> InMemoryUserRepository::remove(const std::string& id) {
  const std::lock_guard<std::mutex> lock(mutex_);
  if (users_by_id_.erase(id) == 0) {
    return tl::unexpected(Error{ErrorKind::kNotFound, "user not found", ""});
  }
  return {};
}

}  // namespace rockvn::user::repository
