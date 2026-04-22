## Arena Memory

### Results (1M objects, 2 threads, -O2)

| Test | Ctor | Dtor |
|---|---|---|
| Global (mutex, inline) | 33,471 µs | 0 µs |
| Meyer's (mutex, inline) | 41,520 µs | 0 µs |
| Atomic (inline) | 18,127 µs | 0 µs |
| Lib Global (mutex) | 44,833 µs | 0 µs |
| Lib Meyer's (mutex) | 39,880 µs | 0 µs |
| Lib Atomic | 23,643 µs | 0 µs |
| Lib Atomic Template | 23,408 µs | 0 µs |

### Key Takeaways

1. **Deallocation is truly zero** — every dtor time is `0 µs`. The no-op `deallocate` costs nothing measurable. This is the core arena trade-off: give up per-object deallocation to make bulk reclaim free via `reset()`.

2. **Atomic is fastest in both variants** — lock-free `fetch_add` beats mutex by ~46% inline and ~47% in the lib version. The mutex serializes two threads onto a single lock; `fetch_add` lets both threads make forward progress simultaneously with a single atomic instruction.

3. **Lib versions are slower than inline** — inline atomic: 18ms → lib atomic: 23ms (+27%). When the arena is in a separate TU, the compiler cannot inline `allocate_one()` into `operator new`. Mutex variants pay a higher penalty because each `new` becomes a chain of out-of-line calls. Atomic loses less because `fetch_add` is a single hardware instruction even without inlining.

4. **Meyer's is slower than global (inline) but equal in lib** — inline: Meyer (41ms) is ~24% slower than global (33ms) because every `get()` call checks the hidden "initialized?" flag the compiler inserts for function-local statics. In the lib version both are ~40-44ms, swamped by out-of-line call overhead.

5. **Header/source split is essential for ODR[One Definition Rule]** — non-template classes must split into a header (declarations) and a `.cpp` (definitions). The static member `Tribe::singleton` must be defined in exactly one TU. Meyer's variant has no static member to define since the singleton lives inside `get()`.

6. **Template inlining helps inside the `.cpp` TU but doesn't cross TU boundaries** — `Lib Atomic Template` inlines `allocate_one()` into `operator new` within its `.cpp`, reducing it to a single `fetch_add`. However the test still calls `operator new` out-of-line, which is the actual bottleneck. Result: 23,408 µs vs 23,643 µs — no measurable difference vs non-template lib atomic.

## Simple Shared Pointer

A hand-rolled `shared_ptr<T>` using an atomic reference count and separate heap-allocated control block.

### Key Takeaways

1. **`const T&&` is almost always a bug** — `const shared_ptr&&` accepts an rvalue but forbids modifying it. Since move semantics exist to *steal* from the source (nulling out `p` and `ctr`), the correct signature is `shared_ptr&&` (non-const). If you see `const T&&`, treat it as a warning sign.

2. **Decrement needs `acq_rel`, increment only needs `relaxed`** — the copy increment just needs atomicity, no ordering. But the decrement that reaches `1 → 0` must *acquire* all writes from other threads (so `delete p` sees a fully constructed object) and *release* its own writes before deleting. Using `relaxed` on the decrement is a data race.

3. **The control block allocation can throw — handle it** — if `new ctr` throws (OOM) after `p` is already constructed, `p` leaks without a catch block. This is why `std::make_shared` is preferred in real code: it allocates the object and control block in one shot, eliminating this window entirely.

4. **Copy-and-swap makes assignment operators trivially correct** — constructing a temporary and calling `swap` gives self-assignment safety and exception safety for free, with no `if (this != &other)` guard needed. The same pattern works for both copy and move assignment.

5. **`std::exchange` is the idiomatic move primitive** — `p{std::exchange(other.p, nullptr)}` reads, nulls out, and returns the old value in one expression. It makes the intent (steal and null out the source) explicit and avoids a separate post-init assignment.

6. **Test destructor behaviour with a live-instance counter** — a `static inline std::atomic<int> alive` incremented in the constructor and decremented in the destructor lets you assert exactly when destruction fires, across threads, without any mocking framework. `EXPECT_EQ(Tracked::alive, 0)` after all owners go out of scope is a clean, direct assertion.

## Simple Unique Pointer

A hand-rolled `unique_ptr<T>` with exclusive ownership, a custom deleter via EBO, and a partial specialisation for arrays.

### Key Takeaways

1. **Move semantics errors come in two flavours — this exercise had both** — `shared_ptr` had `const T&&` (can't steal from const). `unique_ptr`'s array specialisation had `T&` (lvalue ref, silently accepts copies instead of moves). Both compile without error in isolation; the mistake only surfaces when you `std::move` into one. Whenever you write a move constructor or assignment, immediately check that `&&` is present and non-const.

2. **`::value` vs `::value_type` — know your `std::integral_constant` members** — `::value` is the actual `true`/`false` bool; `::value_type` is a type alias for `bool` (always `bool`, never the value). Using `::value_type` where a `constexpr bool` is expected fails to compile on every instantiation. The correct variable template helper always ends in `::value`.

3. **Template parameter scope bugs are silent at definition time** — the base class correctly checked `is_deleter_function_candidate_v<D>`, but the `deleter_type` alias inside the class body checked `<T>` (the pointed-to type) instead. Both compile fine at definition — the mismatch only blows up at instantiation. When two expressions are supposed to test the same condition, read them side-by-side.

4. **EBO (Empty Base Optimization) is how zero-cost deleters work** — the deleter is stored as a base class, not a member. If `D` is a stateless functor (like `default_deleter`), the compiler eliminates its storage entirely — `sizeof(unique_ptr<T>)` equals `sizeof(T*)`. A member field would always cost at least 1 extra byte. This is why `std::unique_ptr` also inherits the deleter.

5. **Partial specialisation is the mechanism for `T[]` support** — `unique_ptr<T[], D>` is a separate class template specialisation that swaps `operator*`/`operator->` for `operator[]` and uses `delete[]` in the deleter. The compiler selects the more specialised template automatically, giving `unique_ptr<int[]>` and `unique_ptr<int>` different behaviour with no runtime overhead.

6. **Two `unique_ptr`s must never own the same raw pointer** — unlike `shared_ptr`, there is no reference count. Giving the same raw pointer to two instances causes a double-free at destruction. The correct way to test equality is to compare independently constructed instances (e.g. both null), never by aliasing raw pointers.

7. **`explicit` constructors block `{val}` aggregate-style array init** — `new T[N]{{1},{2},{3}}` fails when the constructor is `explicit` because each element is copy-initialized from its brace-initializer, and `explicit` forbids that implicit conversion. The fix is direct-initialization: `new T[N]{T{1}, T{2}, T{3}}`.

## Tools
- Sanitizers
  - [Address Sanitizer](https://learn.microsoft.com/en-us/cpp/sanitizers/asan?view=msvc-170)
  - [Memory Sanitizer](https://github.com/google/sanitizers/wiki/memorysanitizer)
  - [Thread Sanitizer](https://github.com/google/sanitizers/wiki/threadsanitizercppmanual)
- Compiler Warnings
  - W4 / Wall
- Static Analysis
  - [Clang Static Analyzer](https://clang-analyzer.llvm.org/)
  - [Coverity](https://www.blackduck.com/static-analysis-tools-sast/coverity.html)
- Hardware Support (MTE)
  - [ARM's Memory Tagging Extension](https://developer.android.com/ndk/guides/arm-mte)
  - [Google’s GWP-ASan](https://developer.android.com/ndk/guides/gwp-asan)
  - [Facebook’s CheckPointer](https://github.com/facebook/folly)
