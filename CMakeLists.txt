cmake_minimum_required(VERSION 3.27)

project(
  rockvn
  VERSION 0.1.0
  DESCRIPTION "Production-inspired microservices backend in C++20"
  LANGUAGES CXX)

# Every preset builds out of source under build/<preset-name>; an in-source
# build pollutes the tree and breaks the multi-preset layout.
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not supported. Configure with a preset: cmake --preset <name>")
endif()

# Project-wide language contract. C++20 exactly: no extensions, no fallback
# to an older standard if a compiler cannot honor it.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

enable_testing()

add_subdirectory(tests/toolchain_smoke)
add_subdirectory(services/user)
