#include "cjl_memory_lane/lib_arena_atomic.hpp"
#include "cjl_memory_lane/lib_arena_atomic_template.hpp"
#include "cjl_memory_lane/lib_arena_global.hpp"
#include "cjl_memory_lane/lib_arena_meyer.hpp"

#include <benchmark/benchmark.h>

#include <barrier>
#include <vector>

// GB's ->Threads(2) keeps both threads alive for the entire benchmark — no
// per-iteration spin-up cost. Each thread owns half the index range so there
// is no data race on the orcs vector.
//
// A std::barrier coordinates the reset between iterations:
//   phase 1 — both threads finish allocating, then barrier
//   phase 2 — thread 0 resets the arena, then barrier
// Only then do both threads resume timing for the next iteration.
static constexpr std::size_t HALF = 490'000; // per thread; 2*HALF < NB_MAX

static void BM_Atomic(benchmark::State& state) {
  static std::vector<cjl::atomic::Orc*> orcs;
  orcs.reserve(2 * HALF);
  static std::barrier bar{2};

  const auto start = state.thread_index() == 0 ? 0uz : HALF;
  const auto end   = state.thread_index() == 0 ? HALF : 2 * HALF;

  for (auto _ : state) {
    for (auto i = start; i != end; ++i)
      benchmark::DoNotOptimize(orcs[i] = new cjl::atomic::Orc);

    bar.arrive_and_wait(); // both threads done allocating
    state.PauseTiming();
    if (state.thread_index() == 0) {
      for (auto orc : orcs) delete orc;
      cjl::atomic::Tribe::get().reset();
    }
    bar.arrive_and_wait(); // reset complete
    state.ResumeTiming();
  }

  // Calculate each allocation speed
  // Comment it out if want timing for entire batch
  state.SetItemsProcessed(state.iterations() * HALF);
}

static void BM_AtomicTemplate(benchmark::State& state) {
  using Tribe = cjl::atomic_tmp::SizeBasedArena<cjl::atomic_tmp::Orc, cjl::atomic_tmp::Orc::NB_MAX>;
  static std::vector<cjl::atomic_tmp::Orc*> orcs(2 * HALF);
  static std::barrier bar{2};

  const auto start = state.thread_index() == 0 ? 0uz : HALF;
  const auto end   = state.thread_index() == 0 ? HALF : 2 * HALF;

  for (auto _ : state) {
    for (auto i = start; i != end; ++i)
      benchmark::DoNotOptimize(orcs[i] = new cjl::atomic_tmp::Orc);

    bar.arrive_and_wait();
    state.PauseTiming();
    if (state.thread_index() == 0) {
      for (auto orc : orcs) delete orc;
      Tribe::get().reset();
    }
    bar.arrive_and_wait();
    state.ResumeTiming();
  }
  state.SetItemsProcessed(state.iterations() * HALF);
}

static void BM_Global(benchmark::State& state) {
  static std::vector<cjl::global::Orc*> orcs(2 * HALF);
  static std::barrier bar{2};

  const auto start = state.thread_index() == 0 ? 0uz : HALF;
  const auto end   = state.thread_index() == 0 ? HALF : 2 * HALF;

  for (auto _ : state) {
    for (auto i = start; i != end; ++i)
      benchmark::DoNotOptimize(orcs[i] = new cjl::global::Orc);

    bar.arrive_and_wait();
    state.PauseTiming();
    if (state.thread_index() == 0) {
      for (auto orc : orcs) delete orc;
      cjl::global::Tribe::get().reset();
    }
    bar.arrive_and_wait();
    state.ResumeTiming();
  }
  state.SetItemsProcessed(state.iterations() * HALF);
}

static void BM_Meyer(benchmark::State& state) {
  static std::vector<cjl::meyer::Orc*> orcs(2 * HALF);
  static std::barrier bar{2};

  const auto start = state.thread_index() == 0 ? 0uz : HALF;
  const auto end   = state.thread_index() == 0 ? HALF : 2 * HALF;

  for (auto _ : state) {
    for (auto i = start; i != end; ++i)
      benchmark::DoNotOptimize(orcs[i] = new cjl::meyer::Orc);

    bar.arrive_and_wait();
    state.PauseTiming();
    if (state.thread_index() == 0) {
      for (auto orc : orcs) delete orc;
      cjl::meyer::Tribe::get().reset();
    }
    bar.arrive_and_wait();
    state.ResumeTiming();
  }
  state.SetItemsProcessed(state.iterations() * HALF);
}

BENCHMARK(BM_Atomic)        ->Threads(2)->UseRealTime();
BENCHMARK(BM_AtomicTemplate)->Threads(2)->UseRealTime();
BENCHMARK(BM_Global)        ->Threads(2)->UseRealTime();
BENCHMARK(BM_Meyer)         ->Threads(2)->UseRealTime();
