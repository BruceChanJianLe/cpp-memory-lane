#pragma once

#include <cassert>
#include <cstddef>
#include <atomic>
#include <cstdlib>

namespace cjl::atomic {
  class Orc {
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

  class Tribe {
    std::byte *p;
    std::atomic<std::byte*> curr;
    static Tribe singleton;

    Tribe() : p{static_cast<std::byte*>(std::malloc(Orc::NB_MAX * sizeof(Orc)))} {
      assert(p);
      curr.store(p, std::memory_order_relaxed);
    }
    Tribe(const Tribe&) = delete;
    Tribe& operator=(const Tribe&) = delete;

  public:
    ~Tribe() {
      std::free(p);
    }

    static auto &get() {
      return singleton;
    }

    void reset() {
      curr.store(p, std::memory_order_relaxed);
    }

    void * allocate_one() {
      return curr.fetch_add(sizeof(Orc), std::memory_order_relaxed);
    }

    void * allocate_n(std::size_t n) {
      return curr.fetch_add(n * sizeof(Orc), std::memory_order_relaxed);
    }

    void deallocate_one(void *) noexcept {}

    void deallocate_n(void *) noexcept {}
  };
} // cjl::atomic
