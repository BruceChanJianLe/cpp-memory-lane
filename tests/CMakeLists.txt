# Link to cjl_memory_lane library
function(add_lib_unit_test name)
  add_executable(test_${name})
  target_sources(test_${name}
    PRIVATE
      test_${name}.cpp
      ../src/cjl_memory_lane/${name}.cpp
    PUBLIC
      FILE_SET HEADERS
      BASE_DIRS ../include/
      FILES
        ../include/cjl_memory_lane/${name}.hpp
  )
  target_link_libraries(test_${name} PRIVATE cjl::${name})
  add_test(NAME test_${name} COMMAND test_${name})
endfunction()

# Local standalone test
function(add_standalone_unit_test name)
  add_executable(${name})
  target_sources(${name} PRIVATE ${name}.cpp)
  target_compile_options(${name} PRIVATE -O2)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

# atomic example also uses the static global method
add_standalone_unit_test(test_arena_atomic)
add_standalone_unit_test(test_arena_global)
add_standalone_unit_test(test_arena_meyer)

add_lib_unit_test(lib_arena_atomic)
add_lib_unit_test(lib_arena_global)
add_lib_unit_test(lib_arena_meyer)
add_lib_unit_test(lib_arena_atomic_template)
