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
  src/api/health_handler.cpp src/api/problem_json.cpp src/api/request_logger.cpp
  src/api/server.cpp)
target_link_libraries(
  user_service_api
  PUBLIC user_service_core Drogon::Drogon
  PRIVATE rockvn::warnings rockvn::sanitizers)

add_executable(user_service src/main.cpp)
target_link_libraries(user_service PRIVATE user_service_api rockvn::warnings rockvn::sanitizers)

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(
  user_service_unit_tests tests/unit/config_test.cpp tests/unit/health_handler_test.cpp
                          tests/unit/logging_test.cpp)
target_compile_definitions(user_service_unit_tests PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(user_service_unit_tests PRIVATE user_service_api GTest::gtest_main
                                                      rockvn::warnings rockvn::sanitizers)
gtest_discover_tests(user_service_unit_tests)

add_executable(user_service_integration_tests tests/integration/health_http_test.cpp)
target_link_libraries(user_service_integration_tests PRIVATE user_service_api GTest::gtest_main
                                                             rockvn::warnings rockvn::sanitizers)
gtest_discover_tests(user_service_integration_tests PROPERTIES LABELS integration)
