// In-process integration tests: a real Drogon server wired by the same
// configure_app the production binary uses, exercised over real HTTP on
// loopback. Drogon's application object is process-global, so the server
// starts once for the whole suite and tests run against it sequentially.

#include <drogon/HttpClient.h>
#include <drogon/drogon.h>
#include <gtest/gtest.h>
#include <spdlog/sinks/ostream_sink.h>

#include <chrono>
#include <future>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

#include "api/clock.h"
#include "api/health_handler.h"
#include "api/request_logger.h"
#include "api/server.h"
#include "config/config.h"
#include "config/logging.h"

namespace {

using namespace std::chrono_literals;

constexpr std::uint16_t kTestPort = 18923;

rockvn::user::config::Config test_config() {
  return rockvn::user::config::Config{
      .http_host = "127.0.0.1",
      .http_port = kTestPort,
      .log_level = "info",
      .log_format = rockvn::user::config::LogFormat::kJson,
      .io_threads = 1,
  };
}

class ServerFixture : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    log_stream_ = new std::ostringstream();
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(*log_stream_);
    logger_ = new rockvn::user::logging::StructuredLogger("user-service", test_config(), sink);
    clock_ = new rockvn::user::api::SystemClock();
    health_ = new rockvn::user::api::HealthHandler("user-service", *clock_, {"0.1.0", "testsha"});
    request_logger_ = new rockvn::user::api::RequestLogger(*logger_, *clock_);

    rockvn::user::api::configure_app(test_config(), *health_, *request_logger_);
    server_thread_ = new std::thread([] { drogon::app().run(); });

    // The server is ready when /health answers; bounded wait, no sleeps on
    // the happy path beyond the poll interval.
    const auto deadline = std::chrono::steady_clock::now() + 10s;
    while (std::chrono::steady_clock::now() < deadline) {
      const auto [result, response] = get("/health");
      if (result == drogon::ReqResult::Ok && response) {
        return;
      }
      std::this_thread::sleep_for(50ms);
    }
    FAIL() << "server did not become ready within 10s";
  }

  static void TearDownTestSuite() {
    drogon::app().quit();
    server_thread_->join();
    delete server_thread_;
    delete request_logger_;
    delete health_;
    delete clock_;
    delete logger_;
    delete log_stream_;
  }

  static std::pair<drogon::ReqResult, drogon::HttpResponsePtr> get(
      const std::string& path, const std::string& request_id = {}) {
    auto client =
        drogon::HttpClient::newHttpClient("http://127.0.0.1:" + std::to_string(kTestPort));
    auto request = drogon::HttpRequest::newHttpRequest();
    request->setPath(path);
    if (!request_id.empty()) {
      request->addHeader("X-Request-Id", request_id);
    }

    auto promise =
        std::make_shared<std::promise<std::pair<drogon::ReqResult, drogon::HttpResponsePtr>>>();
    auto future = promise->get_future();
    client->sendRequest(request,
                        [promise](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
                          promise->set_value({result, resp});
                        });
    if (future.wait_for(5s) != std::future_status::ready) {
      return {drogon::ReqResult::Timeout, nullptr};
    }
    return future.get();
  }

  static Json::Value parse_json(const std::string& text) {
    Json::Value value;
    Json::CharReaderBuilder builder;
    std::string errors;
    std::istringstream stream(text);
    EXPECT_TRUE(Json::parseFromStream(builder, stream, &value, &errors))
        << "not valid JSON: " << text;
    return value;
  }

  static std::string last_log_line() {
    const auto all = log_stream_->str();
    auto end = all.find_last_not_of("\r\n");
    if (end == std::string::npos) {
      return {};
    }
    auto start = all.find_last_of('\n', end);
    start = (start == std::string::npos) ? 0 : start + 1;
    return all.substr(start, end - start + 1);
  }

  static std::ostringstream* log_stream_;
  static rockvn::user::logging::StructuredLogger* logger_;
  static rockvn::user::api::SystemClock* clock_;
  static rockvn::user::api::HealthHandler* health_;
  static rockvn::user::api::RequestLogger* request_logger_;
  static std::thread* server_thread_;
};

std::ostringstream* ServerFixture::log_stream_ = nullptr;
rockvn::user::logging::StructuredLogger* ServerFixture::logger_ = nullptr;
rockvn::user::api::SystemClock* ServerFixture::clock_ = nullptr;
rockvn::user::api::HealthHandler* ServerFixture::health_ = nullptr;
rockvn::user::api::RequestLogger* ServerFixture::request_logger_ = nullptr;
std::thread* ServerFixture::server_thread_ = nullptr;

TEST_F(ServerFixture, HealthReturnsDocumentedContract) {
  const auto [result, response] = get("/health");
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

TEST_F(ServerFixture, SuppliedRequestIdIsEchoed) {
  const auto [result, response] = get("/health", "test-id-123");
  ASSERT_EQ(result, drogon::ReqResult::Ok);
  ASSERT_TRUE(response);
  EXPECT_EQ(response->getHeader("x-request-id"), "test-id-123");
}

TEST_F(ServerFixture, MissingRequestIdIsGenerated) {
  const auto [result, response] = get("/health");
  ASSERT_EQ(result, drogon::ReqResult::Ok);
  ASSERT_TRUE(response);
  EXPECT_FALSE(response->getHeader("x-request-id").empty());
}

TEST_F(ServerFixture, EveryRequestProducesOneParseableLogLine) {
  const auto [result, response] = get("/health", "log-assert-id");
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

TEST_F(ServerFixture, UnknownRouteReturnsProblemJson404) {
  const auto [result, response] = get("/definitely-not-a-route");
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
