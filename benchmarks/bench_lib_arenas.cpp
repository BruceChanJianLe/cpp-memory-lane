#include "cjl_memory_lane/lib_arena_atomic.hpp"
#include "cjl_memory_lane/lib_arena_atomic_template.hpp"
#include "cjl_memory_lane/lib_arena_global.hpp"
#include "cjl_memory_lane/lib_arena_meyer.hpp"

#include <benchmark/benchmark.h>

#include <barrier>
#include <vector>

// per-thread allocation count; NThreads * CHUNK must be < NB_MAX (1'000'000)
static constexpr std::size_t CHUNK = 200'000;

// Each policy exposes the Orc type and a reset() for its arena.
template <typename OrcT, typename TribeT>
struct ArenaPolicy {
  using Orc = OrcT;
  static void reset() { TribeT::get().reset(); }
};

using AtomicPolicy = ArenaPolicy<cjl::atomic::Orc, cjl::atomic::Tribe>;
using GlobalPolicy = ArenaPolicy<cjl::global::Orc, cjl::global::Tribe>;
using MeyerPolicy  = ArenaPolicy<cjl::meyer::Orc,  cjl::meyer::Tribe>;

struct AtomicTmpPolicy {
  using Orc = cjl::atomic_tmp::Orc;
  static void reset() {
    using Arena = cjl::atomic_tmp::SizeBasedArena<Orc, Orc::NB_MAX>;
    Arena::get().reset();
  }
};

// NThreads is a template parameter so each ->Threads(N) variant gets its own
// static barrier and orcs vector (different template instantiation).
//
// Each thread owns orcs[tid*CHUNK .. (tid+1)*CHUNK), no data races.
// std::barrier coordinates the per-iteration reset:
//   phase 1 — all threads finish allocating, then barrier
//   phase 2 — thread 0 resets the arena, then barrier
template <typename Policy, int NThreads>
void BM_Arena(benchmark::State& state) {
  using Orc = typename Policy::Orc;
  static std::vector<Orc*> orcs(NThreads * CHUNK);
  static std::barrier<> bar{NThreads};

  const auto tid   = static_cast<std::size_t>(state.thread_index());
  const auto start = tid * CHUNK;
  const auto end   = start + CHUNK;

  for (auto _ : state) {
    for (auto i = start; i != end; ++i)
      benchmark::DoNotOptimize(orcs[i] = new Orc);

    bar.arrive_and_wait(); // all threads done allocating
    state.PauseTiming();
    if (state.thread_index() == 0) {
      for (auto orc : orcs) delete orc;
      Policy::reset();
    }
    bar.arrive_and_wait(); // reset complete
    state.ResumeTiming();
  }
  state.SetItemsProcessed(state.iterations() * CHUNK);
}

// 2-thread benchmarks
BENCHMARK_TEMPLATE(BM_Arena, AtomicPolicy,    2)->Threads(2)->UseRealTime();
BENCHMARK_TEMPLATE(BM_Arena, AtomicTmpPolicy, 2)->Threads(2)->UseRealTime();
BENCHMARK_TEMPLATE(BM_Arena, GlobalPolicy,    2)->Threads(2)->UseRealTime();
BENCHMARK_TEMPLATE(BM_Arena, MeyerPolicy,     2)->Threads(2)->UseRealTime();
// 4-thread benchmarks
BENCHMARK_TEMPLATE(BM_Arena, AtomicPolicy,    4)->Threads(4)->UseRealTime();
BENCHMARK_TEMPLATE(BM_Arena, AtomicTmpPolicy, 4)->Threads(4)->UseRealTime();
BENCHMARK_TEMPLATE(BM_Arena, GlobalPolicy,    4)->Threads(4)->UseRealTime();
BENCHMARK_TEMPLATE(BM_Arena, MeyerPolicy,     4)->Threads(4)->UseRealTime();
