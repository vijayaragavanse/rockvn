// Full CRUD lifecycle and error paths over real HTTP, per the M2 design's
// acceptance criteria — including the duplicate-email concurrency race.

#include <string>
#include <thread>

#include "server_fixture.h"

namespace {

using rockvn::user::testing::ServerFixture;

class UserCrudTest : public ServerFixture {};

TEST_F(UserCrudTest, FullLifecycle) {
  // Create
  auto [create_result, created] =
      request(drogon::Post, "/users", R"({"email":"lifecycle@example.com","name":"Alice"})");
  ASSERT_EQ(create_result, drogon::ReqResult::Ok);
  ASSERT_TRUE(created);
  ASSERT_EQ(created->statusCode(), drogon::k201Created);
  const auto created_body = parse_json(std::string(created->body()));
  const auto id = created_body["id"].asString();
  ASSERT_FALSE(id.empty());
  EXPECT_EQ(created->getHeader("location"), "/users/" + id);
  EXPECT_EQ(created_body["email"].asString(), "lifecycle@example.com");
  EXPECT_EQ(created_body["name"].asString(), "Alice");
  EXPECT_FALSE(created_body["created_at"].asString().empty());

  // Read
  auto [get_result, fetched] = request(drogon::Get, "/users/" + id);
  ASSERT_EQ(get_result, drogon::ReqResult::Ok);
  ASSERT_EQ(fetched->statusCode(), drogon::k200OK);
  EXPECT_EQ(parse_json(std::string(fetched->body()))["id"].asString(), id);

  // List contains it
  auto [list_result, listed] = request(drogon::Get, "/users");
  ASSERT_EQ(list_result, drogon::ReqResult::Ok);
  const auto users = parse_json(std::string(listed->body()))["users"];
  bool found = false;
  for (const auto& user : users) {
    found = found || user["id"].asString() == id;
  }
  EXPECT_TRUE(found);

  // Rename
  auto [rename_result, renamed] =
      request(drogon::Put, "/users/" + id, R"({"name":"Alice Renamed"})");
  ASSERT_EQ(rename_result, drogon::ReqResult::Ok);
  ASSERT_EQ(renamed->statusCode(), drogon::k200OK);
  EXPECT_EQ(parse_json(std::string(renamed->body()))["name"].asString(), "Alice Renamed");

  // Delete, then it is gone
  auto [delete_result, deleted] = request(drogon::Delete, "/users/" + id);
  ASSERT_EQ(delete_result, drogon::ReqResult::Ok);
  EXPECT_EQ(deleted->statusCode(), drogon::k204NoContent);

  auto [gone_result, gone] = request(drogon::Get, "/users/" + id);
  ASSERT_EQ(gone_result, drogon::ReqResult::Ok);
  EXPECT_EQ(gone->statusCode(), drogon::k404NotFound);
}

TEST_F(UserCrudTest, ValidationErrorIsProblemJson400WithField) {
  auto [result, response] =
      request(drogon::Post, "/users", R"({"email":"not-an-email","name":"Bob"})");
  ASSERT_EQ(result, drogon::ReqResult::Ok);
  ASSERT_EQ(response->statusCode(), drogon::k400BadRequest);
  EXPECT_NE(response->getHeader("content-type").find("application/problem+json"),
            std::string::npos);
  const auto body = parse_json(std::string(response->body()));
  EXPECT_EQ(body["title"].asString(), "Validation Failed");
  EXPECT_EQ(body["field"].asString(), "email");
  EXPECT_NE(body["instance"].asString().find("urn:request-id:"), std::string::npos);
}

TEST_F(UserCrudTest, MalformedJsonBodyIs400) {
  auto [result, response] = request(drogon::Post, "/users", "this is not json");
  ASSERT_EQ(result, drogon::ReqResult::Ok);
  EXPECT_EQ(response->statusCode(), drogon::k400BadRequest);
}

TEST_F(UserCrudTest, MissingUserIs404ProblemJson) {
  auto [result, response] = request(drogon::Get, "/users/does-not-exist");
  ASSERT_EQ(result, drogon::ReqResult::Ok);
  EXPECT_EQ(response->statusCode(), drogon::k404NotFound);
  EXPECT_EQ(parse_json(std::string(response->body()))["title"].asString(), "Not Found");
}

TEST_F(UserCrudTest, DuplicateEmailIs409) {
  auto [first_result, first] =
      request(drogon::Post, "/users", R"({"email":"dup@example.com","name":"First"})");
  ASSERT_EQ(first_result, drogon::ReqResult::Ok);
  ASSERT_EQ(first->statusCode(), drogon::k201Created);

  auto [second_result, second] =
      request(drogon::Post, "/users", R"({"email":"dup@example.com","name":"Second"})");
  ASSERT_EQ(second_result, drogon::ReqResult::Ok);
  EXPECT_EQ(second->statusCode(), drogon::k409Conflict);
  EXPECT_EQ(parse_json(std::string(second->body()))["title"].asString(), "Conflict");
}

// M2 acceptance criterion: two concurrent creates with the same email
// yield exactly one 201 and one 409 — uniqueness is enforced by the
// repository under contention, not by a racy pre-check.
TEST_F(UserCrudTest, ConcurrentDuplicateCreatesYieldOne201AndOne409) {
  const std::string body = R"({"email":"race@example.com","name":"Racer"})";
  drogon::HttpStatusCode status_a{};
  drogon::HttpStatusCode status_b{};

  std::thread thread_a([&] {
    auto [result, response] = request(drogon::Post, "/users", body);
    status_a =
        (result == drogon::ReqResult::Ok && response) ? response->statusCode() : drogon::kUnknown;
  });
  std::thread thread_b([&] {
    auto [result, response] = request(drogon::Post, "/users", body);
    status_b =
        (result == drogon::ReqResult::Ok && response) ? response->statusCode() : drogon::kUnknown;
  });
  thread_a.join();
  thread_b.join();

  const bool one_created = (status_a == drogon::k201Created) != (status_b == drogon::k201Created);
  const bool one_conflict =
      (status_a == drogon::k409Conflict) != (status_b == drogon::k409Conflict);
  EXPECT_TRUE(one_created && one_conflict)
      << "got " << status_a << " and " << status_b << ", expected exactly one 201 and one 409";
}

}  // namespace
