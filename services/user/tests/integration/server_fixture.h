#pragma once

// Shared server fixture for all integration suites. Drogon's application
// object is process-global and cannot restart within a process, so the
// fully-wired server (health + users, production configure_app) starts
// once for the whole test binary; suites run against it sequentially.

#include <drogon/HttpClient.h>
#include <drogon/drogon.h>
#include <gtest/gtest.h>
#include <spdlog/sinks/ostream_sink.h>

#include <chrono>
#include <cstdint>
#include <future>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

#include "api/health_handler.h"
#include "api/request_logger.h"
#include "api/server.h"
#include "api/user_handlers.h"
#include "api/uuid_generator.h"
#include "config/config.h"
#include "config/logging.h"
#include "domain/clock.h"
#include "domain/user_service.h"
#include "repository/in_memory_user_repository.h"

namespace rockvn::user::testing {

constexpr std::uint16_t kTestPort = 18923;

inline config::Config test_config() {
  return config::Config{
      .http_host = "127.0.0.1",
      .http_port = kTestPort,
      .log_level = "info",
      .log_format = config::LogFormat::kJson,
      .io_threads = 1,
  };
}

class ServerFixture : public ::testing::Test {
 public:
  // The process-global server stays up until the process exits; gtest has
  // no "after all suites" hook, so the integration main calls this once
  // after RUN_ALL_TESTS.
  static void shutdown_server() {
    if (wiring_ == nullptr) {
      return;
    }
    drogon::app().quit();
    wiring_->server_thread.join();
    delete wiring_;
    wiring_ = nullptr;
  }

 protected:
  struct Wiring {
    std::ostringstream log_stream;
    std::shared_ptr<spdlog::sinks::ostream_sink_mt> sink =
        std::make_shared<spdlog::sinks::ostream_sink_mt>(log_stream);
    logging::StructuredLogger logger{"user-service", test_config(), sink};
    domain::SystemClock clock;
    api::HealthHandler health{"user-service", clock, {"0.1.0", "testsha"}};
    api::RequestLogger request_logger{logger, clock};
    api::UuidGenerator ids;
    repository::InMemoryUserRepository users_repository;
    domain::UserService users{users_repository, clock, ids};
    api::UserHandlers user_handlers{users};
    std::thread server_thread;
  };

  static void SetUpTestSuite() {
    if (wiring_ != nullptr) {
      return;  // server already running for this process
    }
    wiring_ = new Wiring();
    api::configure_app(test_config(), wiring_->health, wiring_->request_logger,
                       wiring_->user_handlers);
    wiring_->server_thread = std::thread([] { drogon::app().run(); });

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds{10};
    while (std::chrono::steady_clock::now() < deadline) {
      const auto [result, response] = request(drogon::Get, "/health");
      if (result == drogon::ReqResult::Ok && response) {
        return;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds{50});
    }
    FAIL() << "server did not become ready within 10s";
  }

  static std::pair<drogon::ReqResult, drogon::HttpResponsePtr> request(
      drogon::HttpMethod method, const std::string& path, const std::string& json_body = {},
      const std::string& request_id = {}) {
    auto client =
        drogon::HttpClient::newHttpClient("http://127.0.0.1:" + std::to_string(kTestPort));
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(method);
    req->setPath(path);
    if (!json_body.empty()) {
      req->setBody(json_body);
      req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    }
    if (!request_id.empty()) {
      req->addHeader("X-Request-Id", request_id);
    }

    auto promise =
        std::make_shared<std::promise<std::pair<drogon::ReqResult, drogon::HttpResponsePtr>>>();
    auto future = promise->get_future();
    client->sendRequest(req,
                        [promise](drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
                          promise->set_value({result, resp});
                        });
    if (future.wait_for(std::chrono::seconds{5}) != std::future_status::ready) {
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
    const auto all = wiring_->log_stream.str();
    auto end = all.find_last_not_of("\r\n");
    if (end == std::string::npos) {
      return {};
    }
    auto start = all.find_last_of('\n', end);
    start = (start == std::string::npos) ? 0 : start + 1;
    return all.substr(start, end - start + 1);
  }

  static Wiring* wiring_;
};

}  // namespace rockvn::user::testing
