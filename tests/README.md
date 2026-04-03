## Arena Memory

```bash
cd build/tests
echo "=== Global Singleton (mutex) ===" && ./test_arena_global && echo "=== Meyer's Singleton (mutex) ===" && ./test_arena_meyer && echo "=== Atomic ===" && ./test_arena_atomic
```
