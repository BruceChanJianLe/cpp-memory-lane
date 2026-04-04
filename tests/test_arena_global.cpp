#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <chrono>
#include <mutex>
#include <utility>
#include <vector>
#include <thread>
#include <iostream>

class Orc {
  [[maybe_unused]] char name[4]{ 'U', 'R', 'G'};
  [[maybe_unused]] int strength = 100;
  [[maybe_unused]] double smell = 1000.0;

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

  Tribe() : p{static_cast<std::byte *>(std::malloc(Orc::NB_MAX * sizeof(Orc)))} {
    assert(p);
    curr = p;
  }

  Tribe(const Tribe&) = delete;
  Tribe& operator=(const Tribe&) = delete;
  static Tribe singleton;

public:
  ~Tribe() {
    std::free(p);
  }

  static auto &get() {
    return singleton;
  }

  void * allocate() {
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

  void deallocate(void *) noexcept {}

  void deallocate_n(void *) noexcept {}
};

Tribe Tribe::singleton;

void * Orc::operator new(std::size_t) {
  return Tribe::get().allocate();
}

void * Orc::operator new[](std::size_t n) {
  return Tribe::get().allocate_n(n / sizeof(Orc));
}

void Orc::operator delete(void *p) noexcept {
  Tribe::get().deallocate(p);
}

void Orc::operator delete[](void *p) noexcept {
  Tribe::get().deallocate_n(p);
}

template <class F, class ... Args>
  auto test(F f, Args &&... args) {
    using namespace std;
    using namespace std::chrono;

    auto pre = high_resolution_clock::now();
    auto res = f(std::forward<Args>(args)...);
    auto post = high_resolution_clock::now();

    return pair{ res, post - pre };
  }

int main ([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
  std::vector<Orc*> orcs;
  orcs.reserve(Orc::NB_MAX);
  static constexpr int half {Orc::NB_MAX / 2};

  auto [_, dt0] = test([&orcs]{
      std::thread t0([&orcs]{
            for (auto i = 0uz; i != half; ++i) {
              orcs[i] = new Orc;
            }
          });

      std::thread t1([&orcs]{
            for (auto i = half; i != Orc::NB_MAX; ++i) {
              orcs[i] = new Orc;
            }
          });
      t0.join();
      t1.join();
      return sizeof(orcs);
      });

  auto [__, dt1] = test([&orcs]{
        for (auto orc : orcs) {
          delete orc;
        }
        return sizeof(orcs);
      });

  std::cout << "Ctor Global: " <<
    std::chrono::duration_cast<std::chrono::microseconds>(dt0) << std::endl;
  std::cout << "Dtor Global: " <<
    std::chrono::duration_cast<std::chrono::microseconds>(dt1) << std::endl;
  return 0;
}
