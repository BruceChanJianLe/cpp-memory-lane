## Arena Memory

> Defined in a single file
```bash
cd build/tests
echo "=== Global Singleton (mutex) ===" && ./test_arena_global && echo "=== Meyer's Singleton (mutex) ===" && ./test_arena_meyer && echo "=== Atomic ===" && ./test_arena_atomic
```

> Define as a library
```bash
cd build/tests
echo "=== Lib Global Singleton (mutex) ===" && ./test_lib_arena_global && echo "=== Lib Meyer's Singleton (mutex) ===" && ./test_lib_arena_meyer && echo "=== Lib Atomic ===" && ./test_lib_arena_atomic
```
