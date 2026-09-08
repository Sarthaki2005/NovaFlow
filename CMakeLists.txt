cmake_minimum_required(VERSION 3.14)
project(AdvanceThreadPool CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable Google Benchmark's internal tests to prevent the GoogleTest error
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.8.3
)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(bench_pool benchmark.cpp)
target_link_libraries(bench_pool PRIVATE benchmark::benchmark pthread)
