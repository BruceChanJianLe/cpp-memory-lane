include(GoogleTest)

# Link to cjl_memory_lane library
function(add_unit_test name)
  add_executable(${name} ${name}.cpp ../src/common/orc.cpp)
  target_link_libraries(${name} PRIVATE cjl_memory_lane gtest_main)
  gtest_discover_tests(${name})
endfunction()

# Local standalone test
function(add_standalone_unit_test name)
  add_executable(${name} ${name}.cpp)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

# add_unit_test(test_basic_arena)
add_standalone_unit_test(test_arena_atomic)
