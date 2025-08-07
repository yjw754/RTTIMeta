cmake_minimum_required(VERSION 3.14...3.22)

project(RTTIMetaBenchmark LANGUAGES CXX)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

# 添加 Google Benchmark
CPMAddPackage(
  NAME benchmark
  GITHUB_REPOSITORY google/benchmark
  VERSION 1.8.3
  OPTIONS
    "BENCHMARK_ENABLE_TESTING OFF"
    "BENCHMARK_DOWNLOAD_DEPENDENCIES ON"
)

if(TEST_INSTALLED_VERSION)
  find_package(RTTIMeta REQUIRED)
else()
  CPMAddPackage(NAME RTTIMeta SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()

# ---- Create binary ----
file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} RTTIMeta benchmark::benchmark)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 14)

