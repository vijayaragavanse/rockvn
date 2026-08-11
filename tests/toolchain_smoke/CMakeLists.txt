# Proves the toolchain contract end to end: a compiler honoring the C++20
# baseline, GoogleTest resolved through the vcpkg manifest, and CTest wiring
# that both developers and CI execute identically. When this target fails,
# the machine is misconfigured — not the code.

find_package(GTest CONFIG REQUIRED)

add_executable(toolchain_smoke toolchain_smoke_test.cpp)
target_link_libraries(toolchain_smoke PRIVATE GTest::gtest_main rockvn::warnings
                                              rockvn::sanitizers)

include(GoogleTest)
gtest_discover_tests(toolchain_smoke)
