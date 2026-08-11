# User Service — M1 server foundation. Target structure enforces ADR-0005
# constraint 3 mechanically: user_service_core must never link Drogon, so a
# framework type reaching config/ (or, from M2, domain/) is a link error,
# not a review catch.

find_package(Drogon CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)

execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE ROCKVN_GIT_SHA
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
  RESULT_VARIABLE rockvn_git_result)
if(NOT rockvn_git_result EQUAL 0)
  set(ROCKVN_GIT_SHA "unknown")
endif()
configure_file(src/config/build_info.h.in generated/config/build_info.h @ONLY)

# Domain: business logic, framework-free by construction — its link line is
# the architecture (tl-expected only; no Drogon, no jsoncpp, no spdlog).
add_library(user_service_domain STATIC src/domain/user_service.cpp)
target_include_directories(user_service_domain PUBLIC src)
target_link_libraries(
  user_service_domain
  PUBLIC tl::expected
  PRIVATE rockvn::warnings rockvn::sanitizers)

# Storage implementations behind the domain's repository seam.
add_library(user_service_repository STATIC src/repository/in_memory_user_repository.cpp)
target_link_libraries(
  user_service_repository
  PUBLIC user_service_domain
  PRIVATE rockvn::warnings rockvn::sanitizers)

add_library(user_service_core STATIC src/config/config.cpp src/config/logging.cpp)
target_include_directories(user_service_core PUBLIC src ${CMAKE_CURRENT_BINARY_DIR}/generated)
# std::getenv is the intended API; MSVC's C4996 push toward getenv_s is not
# a portability win worth an ifdef.
target_compile_definitions(user_service_core PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(
  user_service_core
  PUBLIC spdlog::spdlog tl::expected JsonCpp::JsonCpp
  PRIVATE rockvn::warnings rockvn::sanitizers)

add_library(
  user_service_api STATIC
  src/api/error_mapping.cpp src/api/health_handler.cpp src/api/problem_json.cpp
  src/api/request_logger.cpp src/api/server.cpp src/api/user_handlers.cpp)
target_link_libraries(
  user_service_api
  PUBLIC user_service_core user_service_domain Drogon::Drogon
  PRIVATE rockvn::warnings rockvn::sanitizers)
# GCC reports a potential null dereference at -O2 and above when it inlines
# drogon's std::any attribute accessor: any_cast through a pointer may
# return null in the general case. It cannot here — the key is checked with
# find() first and is only ever written as the type read back, in one file.
# Scoped to this target so the check stays active everywhere else.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(user_service_api PRIVATE -Wno-null-dereference)
endif()

add_executable(user_service src/main.cpp)
target_link_libraries(user_service PRIVATE user_service_api user_service_repository
                                           rockvn::warnings rockvn::sanitizers)

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(
  user_service_unit_tests
  tests/unit/config_test.cpp
  tests/unit/health_handler_test.cpp
  tests/unit/in_memory_user_repository_test.cpp
  tests/unit/logging_test.cpp
  tests/unit/user_service_test.cpp)
target_compile_definitions(user_service_unit_tests PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(user_service_unit_tests PRIVATE tests/contract)
# gmock_main, not gtest_main + gmock: with vcpkg's dynamic triplet those are
# separate DLLs each carrying a gtest registry — tests register in one and
# main() reads the other, silently discovering zero tests.
target_link_libraries(
  user_service_unit_tests PRIVATE user_service_api user_service_repository GTest::gmock_main
                                  rockvn::warnings rockvn::sanitizers)
gtest_discover_tests(user_service_unit_tests)

# Integration tests use a custom main: Drogon's process-global app must be
# stopped exactly once, after every suite has run.
add_executable(
  user_service_integration_tests
  tests/integration/health_http_test.cpp tests/integration/integration_main.cpp
  tests/integration/user_crud_http_test.cpp)
target_link_libraries(
  user_service_integration_tests PRIVATE user_service_api user_service_repository GTest::gtest
                                         rockvn::warnings rockvn::sanitizers)
gtest_discover_tests(user_service_integration_tests PROPERTIES LABELS integration)
