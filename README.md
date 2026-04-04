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
