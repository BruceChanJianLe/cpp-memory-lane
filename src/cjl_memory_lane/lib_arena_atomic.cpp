#include "cjl_memory_lane/lib_arena_atomic.hpp"

namespace cjl {
  Tribe Tribe::singleton;

  void * Orc::operator new(std::size_t) {
    return Tribe::get().allocate_one();
  }

  void * Orc::operator new[](std::size_t n) {
    return Tribe::get().allocate_n(n / sizeof(Orc));
  }

  void Orc::operator delete(void *p) noexcept {
    Tribe::get().deallocate_one(p);
  }

  void Orc::operator delete[](void *p) noexcept {
    Tribe::get().deallocate_n(p);
  }
} // cjl 
