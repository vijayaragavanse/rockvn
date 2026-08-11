#pragma once

// Contract test suite for domain::UserRepository implementations. The
// suite — not a mock — is what keeps implementations honest: the
// in-memory repository runs it in M2, and the M3 PostgreSQL repository
// must pass the identical suite to be accepted behind the same seam.
//
// Instantiate per implementation:
//   using MyRepoTypes = ::testing::Types<MyRepository>;
//   INSTANTIATE_TYPED_TEST_SUITE_P(MyRepo, UserRepositoryContract, MyRepoTypes);
//
// Implementations needing setup specialize RepositoryFactory<T>.

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>

#include "domain/user_repository.h"

namespace rockvn::user::testing {

template <typename Repository>
struct RepositoryFactory {
  static std::unique_ptr<domain::UserRepository> create() { return std::make_unique<Repository>(); }
};

template <typename Repository>
class UserRepositoryContract : public ::testing::Test {
 protected:
  void SetUp() override { repository_ = RepositoryFactory<Repository>::create(); }

  static domain::User user(const std::string& id, const std::string& email, const std::string& name,
                           int created_offset_s = 0) {
    return domain::User{
        .id = id,
        .email = email,
        .name = name,
        .created_at =
            std::chrono::system_clock::time_point{} + std::chrono::seconds{created_offset_s},
    };
  }

  std::unique_ptr<domain::UserRepository> repository_;
};

TYPED_TEST_SUITE_P(UserRepositoryContract);

TYPED_TEST_P(UserRepositoryContract, InsertThenFindRoundTrips) {
  const auto alice = this->user("id-1", "alice@example.com", "Alice");
  ASSERT_TRUE(this->repository_->insert(alice).has_value());
  const auto found = this->repository_->find_by_id("id-1");
  ASSERT_TRUE(found.has_value());
  EXPECT_EQ(*found, alice);
}

TYPED_TEST_P(UserRepositoryContract, DuplicateEmailIsConflict) {
  ASSERT_TRUE(
      this->repository_->insert(this->user("id-1", "alice@example.com", "Alice")).has_value());
  const auto result = this->repository_->insert(this->user("id-2", "alice@example.com", "Clone"));
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().kind, domain::ErrorKind::kConflict);
}

TYPED_TEST_P(UserRepositoryContract, FindMissingIsEmpty) {
  EXPECT_FALSE(this->repository_->find_by_id("missing").has_value());
}

TYPED_TEST_P(UserRepositoryContract, FindAllIsOrderedByCreationThenId) {
  ASSERT_TRUE(this->repository_->insert(this->user("id-b", "b@example.com", "B", 2)).has_value());
  ASSERT_TRUE(this->repository_->insert(this->user("id-a", "a@example.com", "A", 1)).has_value());
  ASSERT_TRUE(this->repository_->insert(this->user("id-c", "c@example.com", "C", 2)).has_value());

  const auto all = this->repository_->find_all();
  ASSERT_EQ(all.size(), 3);
  EXPECT_EQ(all[0].id, "id-a");
  EXPECT_EQ(all[1].id, "id-b");
  EXPECT_EQ(all[2].id, "id-c");
}

TYPED_TEST_P(UserRepositoryContract, UpdatePersistsChanges) {
  auto alice = this->user("id-1", "alice@example.com", "Alice");
  ASSERT_TRUE(this->repository_->insert(alice).has_value());
  alice.name = "Alice Renamed";
  ASSERT_TRUE(this->repository_->update(alice).has_value());
  EXPECT_EQ(this->repository_->find_by_id("id-1")->name, "Alice Renamed");
}

TYPED_TEST_P(UserRepositoryContract, UpdateMissingIsNotFound) {
  const auto result = this->repository_->update(this->user("missing", "x@example.com", "X"));
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().kind, domain::ErrorKind::kNotFound);
}

TYPED_TEST_P(UserRepositoryContract, RemoveDeletesAndSecondRemoveIsNotFound) {
  ASSERT_TRUE(
      this->repository_->insert(this->user("id-1", "alice@example.com", "Alice")).has_value());
  ASSERT_TRUE(this->repository_->remove("id-1").has_value());
  EXPECT_FALSE(this->repository_->find_by_id("id-1").has_value());

  const auto second = this->repository_->remove("id-1");
  ASSERT_FALSE(second.has_value());
  EXPECT_EQ(second.error().kind, domain::ErrorKind::kNotFound);
}

REGISTER_TYPED_TEST_SUITE_P(UserRepositoryContract, InsertThenFindRoundTrips,
                            DuplicateEmailIsConflict, FindMissingIsEmpty,
                            FindAllIsOrderedByCreationThenId, UpdatePersistsChanges,
                            UpdateMissingIsNotFound, RemoveDeletesAndSecondRemoveIsNotFound);

}  // namespace rockvn::user::testing
