cmake_minimum_required(VERSION 3.20)
project(cpp-memory-lane CXX)

# Only add if not defined
if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 23)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/FetchDependencies.cmake)

set(MY_LIB_NAME cjl_memory_lane)

add_library(${MY_LIB_NAME} INTERFACE)
target_include_directories(${MY_LIB_NAME} INTERFACE include/)
target_compile_features(${MY_LIB_NAME} INTERFACE cxx_std_${CMAKE_CXX_STANDARD})
target_compile_options(${MY_LIB_NAME} INTERFACE -Wall -Wextra -fsanitize=address,undefined)
target_link_options(${MY_LIB_NAME} INTERFACE -fsanitize=address,undefined)

# Testing (ensure correctness)
include(CTest)
enable_testing()
add_subdirectory(tests)

# Benchmarking (always measure!)
add_subdirectory(benchmarks)
