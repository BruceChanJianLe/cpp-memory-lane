#pragma once

#include <cassert>
#include <cstddef>
#include <mutex>
#include <cstdlib>

namespace cjl {

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
    std::mutex m;
    std::byte *p, *curr;
    static Tribe singleton;

    Tribe() : p{static_cast<std::byte*>(std::malloc(Orc::NB_MAX * sizeof(Orc)))} {
      assert(p);
      curr = p;
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

    void * allocate_one() {
      std::lock_guard _{m};
      auto q = curr;
      curr += sizeof(Orc);
      return q;
    }

    void * allocate_n(std::size_t n) {
      std::lock_guard _{m};
      auto q = curr;
      curr += n * sizeof(Orc);
      return q;
    }

    void deallocate_one(void *) noexcept {}

    void deallocate_n(void *) noexcept {}
  };
} // cjl
