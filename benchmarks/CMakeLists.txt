# Arena benchmark
add_executable(bench_lib_arenas
  bench_lib_arenas.cpp
  ../src/cjl_memory_lane/lib_arena_atomic.cpp
  ../src/cjl_memory_lane/lib_arena_atomic_template.cpp
  ../src/cjl_memory_lane/lib_arena_global.cpp
  ../src/cjl_memory_lane/lib_arena_meyer.cpp
)
target_link_libraries(bench_lib_arenas PRIVATE
  cjl::lib_arena_atomic
  cjl::lib_arena_atomic_template
  cjl::lib_arena_global
  cjl::lib_arena_meyer
  benchmark::benchmark_main
)
target_compile_options(bench_lib_arenas PRIVATE -O2)

# simple_vector benchmark
add_executable(bench_lib_simple_vector
  bench_lib_simple_vector.cpp
  ../src/cjl_memory_lane/lib_simple_vector.cpp
)
target_link_libraries(bench_lib_simple_vector PRIVATE
  cjl::lib_simple_vector
  benchmark::benchmark_main
)
target_compile_options(bench_lib_simple_vector PRIVATE -O2)
