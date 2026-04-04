#pragma once

#include <cassert>
#include <cstddef>
#include <atomic>
#include <cstdlib>
#include <type_traits>

namespace cjl {

  class Orc final {
    char name[4] {'U', 'R', 'G'};
    int strength = 100;
    double smell = 1000.0;

  public:
    static constexpr int NB_MAX = 1'000'000;
    void * operator new(std::size_t);
    void * operator new[](std::size_t);
    void operator delete(void *) noexcept;
    void operator delete[](void *) noexcept;
  };

  template <class T, std::size_t N>
  class SizeBasedArena {
    static_assert(std::is_final_v<T>);
    std::byte *p;
    std::atomic<std::byte*> curr;
    static SizeBasedArena singleton;

    SizeBasedArena() : p{static_cast<std::byte*>(std::malloc(N * sizeof(T)))} {
      assert(p);
      curr.store(p, std::memory_order_relaxed);
    }
    SizeBasedArena(const SizeBasedArena&) = delete;
    SizeBasedArena& operator=(const SizeBasedArena&) = delete;

  public:
    ~SizeBasedArena() {
      std::free(p);
    }

    static auto &get() {
      return singleton;
    }

    void * allocate_one() {
      return curr.fetch_add(sizeof(T), std::memory_order_relaxed);
    }

    void * allocate_n(std::size_t n) {
      return curr.fetch_add(n * sizeof(T), std::memory_order_relaxed);
    }

    void deallocate_one(void *) noexcept {}

    void deallocate_n(void *) noexcept {}
  };
} // cjl
