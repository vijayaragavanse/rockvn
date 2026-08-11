#include "domain/user_service.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <string>

#include "domain/clock.h"
#include "domain/id_generator.h"
#include "domain/user_repository.h"

namespace {

using namespace std::chrono_literals;
using ::testing::_;
using ::testing::Return;

using rockvn::user::domain::Error;
using rockvn::user::domain::ErrorKind;
using rockvn::user::domain::User;
using rockvn::user::domain::UserService;

class MockUserRepository : public rockvn::user::domain::UserRepository {
 public:
  MOCK_METHOD((tl::expected<void, Error>), insert, (const User&), (override));
  MOCK_METHOD(std::optional<User>, find_by_id, (const std::string&), (const, override));
  MOCK_METHOD(std::vector<User>, find_all, (), (const, override));
  MOCK_METHOD((tl::expected<void, Error>), update, (const User&), (override));
  MOCK_METHOD((tl::expected<void, Error>), remove, (const std::string&), (override));
};

class FixedClock final : public rockvn::user::domain::Clock {
 public:
  std::chrono::system_clock::time_point system_now() const override {
    return std::chrono::sys_days{std::chrono::year{2026} / 8 / 11} + 10h;
  }
  std::chrono::steady_clock::time_point steady_now() const override { return {}; }
};

class FixedIdGenerator final : public rockvn::user::domain::IdGenerator {
 public:
  std::string generate() const override { return "fixed-id-1"; }
};

class UserServiceTest : public ::testing::Test {
 protected:
  MockUserRepository repository_;
  FixedClock clock_;
  FixedIdGenerator ids_;
  UserService service_{repository_, clock_, ids_};
};

TEST_F(UserServiceTest, CreateValidatesAssignsIdAndTimestampsThenInserts) {
  User inserted;
  EXPECT_CALL(repository_, insert(_)).WillOnce([&inserted](const User& user) {
    inserted = user;
    return tl::expected<void, Error>{};
  });

  const auto result = service_.create("alice@example.com", "Alice");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, "fixed-id-1");
  EXPECT_EQ(result->email, "alice@example.com");
  EXPECT_EQ(result->name, "Alice");
  EXPECT_EQ(result->created_at, clock_.system_now());
  EXPECT_EQ(inserted, *result);
}

TEST_F(UserServiceTest, CreateRejectsInvalidEmailWithoutTouchingStorage) {
  EXPECT_CALL(repository_, insert(_)).Times(0);
  for (const auto* email : {"", "no-at-sign", "@nolocal", "notail@", "two@@ats", "sp ace@x.com"}) {
    const auto result = service_.create(email, "Alice");
    ASSERT_FALSE(result.has_value()) << "accepted: " << email;
    EXPECT_EQ(result.error().kind, ErrorKind::kValidation);
    EXPECT_EQ(result.error().field, "email");
  }
}

TEST_F(UserServiceTest, CreateRejectsInvalidNameWithoutTouchingStorage) {
  EXPECT_CALL(repository_, insert(_)).Times(0);
  const std::string too_long(101, 'x');
  for (const std::string& name : {std::string{}, std::string{"   "}, too_long}) {
    const auto result = service_.create("alice@example.com", name);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().kind, ErrorKind::kValidation);
    EXPECT_EQ(result.error().field, "name");
  }
}

TEST_F(UserServiceTest, CreatePropagatesConflictFromRepository) {
  EXPECT_CALL(repository_, insert(_))
      .WillOnce(
          Return(tl::unexpected(Error{ErrorKind::kConflict, "email already registered", "email"})));
  const auto result = service_.create("alice@example.com", "Alice");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().kind, ErrorKind::kConflict);
}

TEST_F(UserServiceTest, GetMissingIsNotFound) {
  EXPECT_CALL(repository_, find_by_id("missing")).WillOnce(Return(std::nullopt));
  const auto result = service_.get("missing");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().kind, ErrorKind::kNotFound);
}

TEST_F(UserServiceTest, RenameValidatesLoadsMutatesAndUpdates) {
  const User existing{.id = "id-1",
                      .email = "alice@example.com",
                      .name = "Alice",
                      .created_at = clock_.system_now()};
  EXPECT_CALL(repository_, find_by_id("id-1")).WillOnce(Return(existing));

  User updated;
  EXPECT_CALL(repository_, update(_)).WillOnce([&updated](const User& user) {
    updated = user;
    return tl::expected<void, Error>{};
  });

  const auto result = service_.rename("id-1", "Alice Renamed");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->name, "Alice Renamed");
  EXPECT_EQ(updated.name, "Alice Renamed");
  EXPECT_EQ(updated.email, existing.email);  // email is immutable through rename
}

TEST_F(UserServiceTest, RenameMissingIsNotFound) {
  EXPECT_CALL(repository_, find_by_id("missing")).WillOnce(Return(std::nullopt));
  const auto result = service_.rename("missing", "New Name");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error().kind, ErrorKind::kNotFound);
}

TEST_F(UserServiceTest, RemoveDelegatesToRepository) {
  EXPECT_CALL(repository_, remove("id-1")).WillOnce(Return(tl::expected<void, Error>{}));
  EXPECT_TRUE(service_.remove("id-1").has_value());
}

}  // namespace
