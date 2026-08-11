#pragma once

#include <chrono>

namespace rockvn::user::api {

// The one seam M1 needs a test double for: time. system_now feeds
// human-facing timestamps; steady_now feeds durations and uptime, immune
// to wall-clock adjustments.
class Clock {
 public:
  virtual ~Clock() = default;
  virtual std::chrono::system_clock::time_point system_now() const = 0;
  virtual std::chrono::steady_clock::time_point steady_now() const = 0;
};

class SystemClock final : public Clock {
 public:
  std::chrono::system_clock::time_point system_now() const override {
    return std::chrono::system_clock::now();
  }
  std::chrono::steady_clock::time_point steady_now() const override {
    return std::chrono::steady_clock::now();
  }
};

}  // namespace rockvn::user::api
