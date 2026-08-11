#include "api/health_handler.h"

#include <gtest/gtest.h>

#include <chrono>

#include "api/clock.h"

namespace {

using namespace std::chrono_literals;

class FakeClock final : public rockvn::user::api::Clock {
 public:
  std::chrono::system_clock::time_point system_now() const override { return system_now_; }
  std::chrono::steady_clock::time_point steady_now() const override { return steady_now_; }

  void advance(std::chrono::seconds delta) {
    system_now_ += delta;
    steady_now_ += delta;
  }

 private:
  // 2026-01-02T03:04:05Z as a known, assertable instant.
  std::chrono::system_clock::time_point system_now_{
      std::chrono::sys_days{std::chrono::year{2026} / 1 / 2} + 3h + 4min + 5s};
  std::chrono::steady_clock::time_point steady_now_{std::chrono::steady_clock::duration{0}};
};

TEST(HealthHandler, ReportsStatusServiceVersionUptimeAndTimestamp) {
  FakeClock clock;
  const rockvn::user::api::HealthHandler handler("user-service", clock, {"1.2.3", "abc1234"});

  clock.advance(42s);
  const auto body = handler.status();

  EXPECT_EQ(body["status"].asString(), "ok");
  EXPECT_EQ(body["service"].asString(), "user-service");
  EXPECT_EQ(body["version"].asString(), "1.2.3+gabc1234");
  EXPECT_EQ(body["uptime_seconds"].asInt64(), 42);
  EXPECT_EQ(body["timestamp"].asString(), "2026-01-02T03:04:47.000Z");
}

TEST(HealthHandler, UptimeStartsAtZero) {
  FakeClock clock;
  const rockvn::user::api::HealthHandler handler("user-service", clock, {"1.2.3", "abc1234"});
  EXPECT_EQ(handler.status()["uptime_seconds"].asInt64(), 0);
}

}  // namespace
