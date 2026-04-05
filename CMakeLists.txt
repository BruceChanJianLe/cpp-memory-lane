cmake_minimum_required(VERSION 3.20)
project(cpp-memory-lane CXX)

# Only add if not defined
if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 23)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# include(cmake/FetchDependencies.cmake)


function(add_lib name)
add_library(${name} INTERFACE)
add_library(cjl::${name} ALIAS ${name})
target_sources(${name}
  PRIVATE
    ./src/cjl_memory_lane/${name}.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS include/
    FILES
      ./include/cjl_memory_lane/${name}.hpp
)
target_compile_features(${name} INTERFACE cxx_std_${CMAKE_CXX_STANDARD})
target_compile_options(${name} INTERFACE -Wall -Wextra -O2)
endfunction()

add_lib(lib_arena_atomic)
add_lib(lib_arena_global)
add_lib(lib_arena_meyer)
add_lib(lib_arena_atomic_template)

# Testing (ensure correctness)
include(CTest)
enable_testing()
add_subdirectory(tests)

# Benchmarking (always measure!)
add_subdirectory(benchmarks)
