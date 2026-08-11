// Verifies that the active toolchain honors the C++20 baseline this
// repository assumes: concepts, ranges, designated initializers, and
// defaulted three-way comparison. A failure here means the machine or CI
// image is misconfigured, not that production code is broken.

#include <gtest/gtest.h>

#include <numeric>
#include <ranges>
#include <string>
#include <vector>

namespace {

template <typename T>
concept Summable = requires(T lhs, T rhs) {
  { lhs + rhs } -> std::convertible_to<T>;
};

template <Summable T>
T sum(const std::vector<T>& values, T initial) {
  return std::accumulate(values.begin(), values.end(), std::move(initial));
}

struct Endpoint {
  std::string host;
  int port{0};

  friend auto operator<=>(const Endpoint&, const Endpoint&) = default;
};

TEST(ToolchainSmoke, ConceptsConstrainTemplates) {
  static_assert(Summable<int>);
  static_assert(Summable<std::string>);
  static_assert(!Summable<Endpoint>);
  EXPECT_EQ(sum<int>({1, 2, 3}, 0), 6);
}

TEST(ToolchainSmoke, RangesComposeLazily) {
  const std::vector<int> values{1, 2, 3, 4, 5, 6};
  auto evens = values | std::views::filter([](int value) { return value % 2 == 0; });
  EXPECT_EQ(std::accumulate(evens.begin(), evens.end(), 0), 12);
}

TEST(ToolchainSmoke, DesignatedInitializersAndThreeWayComparison) {
  const auto primary = Endpoint{.host = "localhost", .port = 5432};
  const auto replica = Endpoint{.host = "localhost", .port = 5433};
  EXPECT_LT(primary, replica);
  EXPECT_EQ(primary, (Endpoint{.host = "localhost", .port = 5432}));
}

}  // namespace
