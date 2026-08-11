#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "domain/user_repository.h"

namespace rockvn::user::repository {

// M2's storage: proves the repository seam before a database complicates
// it (M3 swaps in PostgreSQL behind the same interface and contract
// tests). Mutex-guarded — correct under any io_threads setting; anything
// cleverer is optimization without a measurement.
class InMemoryUserRepository final : public domain::UserRepository {
 public:
  tl::expected<void, domain::Error> insert(const domain::User& user) override;
  std::optional<domain::User> find_by_id(const std::string& id) const override;
  std::vector<domain::User> find_all() const override;
  tl::expected<void, domain::Error> update(const domain::User& user) override;
  tl::expected<void, domain::Error> remove(const std::string& id) override;

 private:
  mutable std::mutex mutex_;
  std::unordered_map<std::string, domain::User> users_by_id_;
};

}  // namespace rockvn::user::repository
