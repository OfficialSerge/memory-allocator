cmake_minimum_required(VERSION 4.0)

project(STACK DESCRIPTION "a stack" LANGUAGES CXX VERSION 1.0)

include(cmake/UpdateSubmodules.cmake)
include(CTest)
include(GoogleTest)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(external)

option(STACK_EXAMPLES "Build my examples" OFF)
option(STACK_BENCHMARKS "Build my benchmarks" OFF)

add_library(target_stack INTERFACE)
add_library(stack::stack ALIAS target_stack)

target_sources(target_stack
  INTERFACE
    FILE_SET HEADERS
    BASE_DIRS source/include
    FILES
      source/include/stack.hpp
      source/include/arena_stack.hpp
)

set_target_properties(
  target_stack PROPERTIES
    VERIFY_INTERFACE_HEADER_SETS ON
)

if(STACK_EXAMPLES)
  add_executable(examples)
  target_sources(examples PRIVATE examples/example.cpp)
  target_link_libraries(examples PRIVATE stack::stack)
endif()

if(STACK_BENCHMARKS)
  add_executable(benchmark_stack)
  target_sources(benchmark_stack PRIVATE benchmarks/benchmark.cpp)
  target_link_libraries(benchmark_stack PRIVATE stack::stack benchmark::benchmark_main)

  add_executable(benchmark_arena_stack)
  target_sources(benchmark_arena_stack PRIVATE benchmarks/benchmark_arena_stack.cpp)
  target_link_libraries(benchmark_arena_stack PRIVATE stack::stack benchmark::benchmark_main)

endif()

if(BUILD_TESTING)
  add_executable(tests)
  target_sources(tests PRIVATE tests/test.cpp)
  target_link_libraries(tests PRIVATE stack::stack GTest::gtest_main)
  gtest_discover_tests(tests)

  add_executable(arena_tests)
  target_sources(arena_tests PRIVATE tests/arena_test.cpp)
  target_link_libraries(arena_tests PRIVATE stack::stack GTest::gtest_main)
  gtest_discover_tests(arena_tests)
endif()
