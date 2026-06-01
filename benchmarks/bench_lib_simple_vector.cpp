#include "cjl_memory_lane/lib_simple_vector.hpp"

#include <benchmark/benchmark.h>

#include <string>
#include <type_traits>
#include <vector>

static constexpr int N = 1'000'000;

// SSO threshold is typically 15 chars on libstdc++/libc++
static const std::string SSO_VAL  = "hello";
static const std::string HEAP_VAL = "this-string-is-long-enough-to-escape-sso-and-land-on-the-heap";

// ── acc: uniform sink for int and std::string ────────────────────────────────

template <class T>
static std::size_t acc(const T &x) {
  if constexpr (std::is_same_v<T, std::string>)
    return x.size();
  else
    return static_cast<std::size_t>(x);
}

// ── push_back from empty ─────────────────────────────────────────────────────
//
// cjl::simple::vector grows via new T[new_cap], default-constructing ALL
// capacity slots (including unused ones), then copy/move-assigning live
// elements. std::vector uses raw memory + placement new for live elements only.
// The gap is visible for non-trivial T (string), negligible for int.

template <class Vec, class T>
static void do_push_back(benchmark::State &state, const T &val) {
  for (auto _ : state) {
    Vec v;
    for (int i = 0; i < N; ++i)
      v.push_back(val);
    benchmark::DoNotOptimize(v);
  }
  state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Simple_PushBack_Int(benchmark::State &s)      { do_push_back<cjl::simple::vector<int>>(s, 42); }
static void BM_Std_PushBack_Int(benchmark::State &s)         { do_push_back<std::vector<int>>(s, 42); }
static void BM_Simple_PushBack_StrSSO(benchmark::State &s)   { do_push_back<cjl::simple::vector<std::string>>(s, SSO_VAL); }
static void BM_Std_PushBack_StrSSO(benchmark::State &s)      { do_push_back<std::vector<std::string>>(s, SSO_VAL); }
static void BM_Simple_PushBack_StrHeap(benchmark::State &s)  { do_push_back<cjl::simple::vector<std::string>>(s, HEAP_VAL); }
static void BM_Std_PushBack_StrHeap(benchmark::State &s)     { do_push_back<std::vector<std::string>>(s, HEAP_VAL); }

BENCHMARK(BM_Simple_PushBack_Int);
BENCHMARK(BM_Std_PushBack_Int);
BENCHMARK(BM_Simple_PushBack_StrSSO);
BENCHMARK(BM_Std_PushBack_StrSSO);
BENCHMARK(BM_Simple_PushBack_StrHeap);
BENCHMARK(BM_Std_PushBack_StrHeap);

// ── iterate over N elements ──────────────────────────────────────────────────
//
// Once built, the memory layout of live elements is the same for both vectors.
// Iteration throughput should be identical; any gap here is measurement noise.

template <class Vec, class T>
static void do_iterate(benchmark::State &state, const T &val) {
  Vec v;
  for (int i = 0; i < N; ++i) v.push_back(val);
  for (auto _ : state) {
    std::size_t sum = 0;
    for (const auto &x : v) sum += acc(x);
    benchmark::DoNotOptimize(sum);
  }
  state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Simple_Iterate_Int(benchmark::State &s)      { do_iterate<cjl::simple::vector<int>>(s, 42); }
static void BM_Std_Iterate_Int(benchmark::State &s)         { do_iterate<std::vector<int>>(s, 42); }
static void BM_Simple_Iterate_StrSSO(benchmark::State &s)   { do_iterate<cjl::simple::vector<std::string>>(s, SSO_VAL); }
static void BM_Std_Iterate_StrSSO(benchmark::State &s)      { do_iterate<std::vector<std::string>>(s, SSO_VAL); }
static void BM_Simple_Iterate_StrHeap(benchmark::State &s)  { do_iterate<cjl::simple::vector<std::string>>(s, HEAP_VAL); }
static void BM_Std_Iterate_StrHeap(benchmark::State &s)     { do_iterate<std::vector<std::string>>(s, HEAP_VAL); }

BENCHMARK(BM_Simple_Iterate_Int);
BENCHMARK(BM_Std_Iterate_Int);
BENCHMARK(BM_Simple_Iterate_StrSSO);
BENCHMARK(BM_Std_Iterate_StrSSO);
BENCHMARK(BM_Simple_Iterate_StrHeap);
BENCHMARK(BM_Std_Iterate_StrHeap);

// ── copy construction ─────────────────────────────────────────────────────────
//
// cjl copy ctor: new T[src.size()] (default-constructs all N slots) then
// copy-assigns each. std copy ctor: allocates raw memory then copy-constructs
// each element directly. For string the extra default-construct step is cheap
// (SSO empty string) but measurable at scale.

template <class Vec, class T>
static void do_copy_ctor(benchmark::State &state, const T &val) {
  Vec src;
  for (int i = 0; i < N; ++i) src.push_back(val);
  for (auto _ : state) {
    Vec copy{src};
    benchmark::DoNotOptimize(copy);
  }
  state.SetItemsProcessed(state.iterations() * N);
}

static void BM_Simple_CopyCtor_Int(benchmark::State &s)      { do_copy_ctor<cjl::simple::vector<int>>(s, 42); }
static void BM_Std_CopyCtor_Int(benchmark::State &s)         { do_copy_ctor<std::vector<int>>(s, 42); }
static void BM_Simple_CopyCtor_StrSSO(benchmark::State &s)   { do_copy_ctor<cjl::simple::vector<std::string>>(s, SSO_VAL); }
static void BM_Std_CopyCtor_StrSSO(benchmark::State &s)      { do_copy_ctor<std::vector<std::string>>(s, SSO_VAL); }
static void BM_Simple_CopyCtor_StrHeap(benchmark::State &s)  { do_copy_ctor<cjl::simple::vector<std::string>>(s, HEAP_VAL); }
static void BM_Std_CopyCtor_StrHeap(benchmark::State &s)     { do_copy_ctor<std::vector<std::string>>(s, HEAP_VAL); }

BENCHMARK(BM_Simple_CopyCtor_Int);
BENCHMARK(BM_Std_CopyCtor_Int);
BENCHMARK(BM_Simple_CopyCtor_StrSSO);
BENCHMARK(BM_Std_CopyCtor_StrSSO);
BENCHMARK(BM_Simple_CopyCtor_StrHeap);
BENCHMARK(BM_Std_CopyCtor_StrHeap);
