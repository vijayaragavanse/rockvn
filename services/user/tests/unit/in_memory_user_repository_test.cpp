// The in-memory repository is validated by the shared contract suite —
// the same suite the M3 PostgreSQL implementation must pass. The
// instantiation lives in the suite's namespace because gtest's typed-test
// macros paste the suite name into generated identifiers.

#include "repository/in_memory_user_repository.h"

#include <gtest/gtest.h>

#include "user_repository_contract.h"

namespace rockvn::user::testing {

using InMemoryTypes = ::testing::Types<repository::InMemoryUserRepository>;
INSTANTIATE_TYPED_TEST_SUITE_P(InMemory, UserRepositoryContract, InMemoryTypes);

}  // namespace rockvn::user::testing
