// Custom test main: gtest has no after-all-suites hook, and Drogon's
// process-global app must be stopped exactly once, after every suite ran.

#include <gtest/gtest.h>

#include "server_fixture.h"

namespace rockvn::user::testing {
ServerFixture::Wiring* ServerFixture::wiring_ = nullptr;
}  // namespace rockvn::user::testing

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  const int result = RUN_ALL_TESTS();
  rockvn::user::testing::ServerFixture::shutdown_server();
  return result;
}
