#include "cjl_memory_lane/lib_arena_global.hpp"

#include <cassert>
#include <cstdlib>
#include <chrono>
#include <utility>
#include <vector>
#include <thread>
#include <iostream>

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
  using namespace cjl;
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

  std::cout << "Ctor Lib Global: " <<
    std::chrono::duration_cast<std::chrono::microseconds>(dt0) << std::endl;
  std::cout << "Dtor Lib Global: " <<
    std::chrono::duration_cast<std::chrono::microseconds>(dt1) << std::endl;
  return 0;
}
