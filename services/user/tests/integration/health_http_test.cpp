// Health-endpoint and observability contracts over real HTTP, using the
// shared fully-wired server fixture.

#include <string>

#include "server_fixture.h"

namespace {

using rockvn::user::testing::ServerFixture;

class HealthHttpTest : public ServerFixture {};

TEST_F(HealthHttpTest, HealthReturnsDocumentedContract) {
  const auto [result, response] = request(drogon::Get, "/health");
  ASSERT_EQ(result, drogon::ReqResult::Ok);
  ASSERT_TRUE(response);
  EXPECT_EQ(response->statusCode(), drogon::k200OK);

  const auto body = parse_json(std::string(response->body()));
  EXPECT_EQ(body["status"].asString(), "ok");
  EXPECT_EQ(body["service"].asString(), "user-service");
  EXPECT_EQ(body["version"].asString(), "0.1.0+gtestsha");
  EXPECT_TRUE(body.isMember("uptime_seconds"));
  EXPECT_FALSE(body["timestamp"].asString().empty());
}

TEST_F(HealthHttpTest, SuppliedRequestIdIsEchoed) {
  const auto [result, response] = request(drogon::Get, "/health", {}, "test-id-123");
  ASSERT_EQ(result, drogon::ReqResult::Ok);
  ASSERT_TRUE(response);
  EXPECT_EQ(response->getHeader("x-request-id"), "test-id-123");
}

TEST_F(HealthHttpTest, MissingRequestIdIsGenerated) {
  const auto [result, response] = request(drogon::Get, "/health");
  ASSERT_EQ(result, drogon::ReqResult::Ok);
  ASSERT_TRUE(response);
  EXPECT_FALSE(response->getHeader("x-request-id").empty());
}

TEST_F(HealthHttpTest, EveryRequestProducesOneParseableLogLine) {
  const auto [result, response] = request(drogon::Get, "/health", {}, "log-assert-id");
  ASSERT_EQ(result, drogon::ReqResult::Ok);

  const auto line = last_log_line();
  ASSERT_FALSE(line.empty());
  const auto parsed = parse_json(line);
  EXPECT_EQ(parsed["event"].asString(), "http_request");
  EXPECT_EQ(parsed["method"].asString(), "GET");
  EXPECT_EQ(parsed["path"].asString(), "/health");
  EXPECT_EQ(parsed["status"].asInt(), 200);
  EXPECT_GE(parsed["duration_ms"].asDouble(), 0.0);
  EXPECT_EQ(parsed["request_id"].asString(), "log-assert-id");
}

TEST_F(HealthHttpTest, UnknownRouteReturnsProblemJson404) {
  const auto [result, response] = request(drogon::Get, "/definitely-not-a-route");
  ASSERT_EQ(result, drogon::ReqResult::Ok);
  ASSERT_TRUE(response);
  EXPECT_EQ(response->statusCode(), drogon::k404NotFound);
  EXPECT_NE(response->getHeader("content-type").find("application/problem+json"),
            std::string::npos);

  const auto body = parse_json(std::string(response->body()));
  EXPECT_EQ(body["title"].asString(), "Not Found");
  EXPECT_EQ(body["status"].asInt(), 404);
}

}  // namespace
