#include "cjl_memory_lane/lib_simple_vector.hpp"

#include <chrono>
#include <concepts>
#include <print>

template <class F, class... Args> auto test(F f, Args &&...args) {
  using namespace std;
  using namespace std::chrono;

  auto pre = high_resolution_clock::now();
  auto res = f(std::forward<Args>(args)...);
  auto post = high_resolution_clock::now();

  return std::pair{res, post - pre};
}

template <class Vec>
concept ContainerInt = requires(Vec v, int val) {
  {v.push_back(val)} -> std::same_as<void>;
  {v.size()} -> std::convertible_to<std::size_t>;
};

template <ContainerInt Vec, int size>
int do_push_back(){
  Vec v;
  for (auto i = 0; i < size; ++i) {
    v.push_back(i);
  }
  return v.size();
}

template <class Vec>
concept ContainerStr = requires(Vec v, std::string val) {
  {v.push_back(val)} -> std::same_as<void>;
  {v.size()} -> std::convertible_to<std::size_t>;
};

template <ContainerStr Vec, int size>
int do_push_back(){
  Vec v;
  for (auto i = 0; i < size; ++i) {
    v.push_back("I love this example, strange as it is!");
  }
  return v.size();
}
int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
  using namespace std;
  using namespace std::chrono;

  const static constexpr int N0 = 1'000'000;
  const static constexpr int N1 = 100'000;

  auto [r0, dt0] = test([] -> int {
      return do_push_back<cjl::simple::vector<int>, N0>();
  });

  auto [r1, dt1] = test([] -> int {
      return do_push_back<std::vector<int>, N0>();
  });

  auto [r2, dt2] = test([] -> int {
      return do_push_back<cjl::simple::vector<string>, N1>();
  });

  auto [r3, dt3] = test([] -> int {
      return do_push_back<std::vector<string>, N1>();
  });

  print("Vector<int>,         push_back(), {} times: {} in {}\n",
      N0, r0, duration_cast<microseconds>(dt0));
  print("std::vector<int>,    push_back(), {} times: {} in {}\n",
      N0, r1, duration_cast<microseconds>(dt1));
  print("Vector<string>,      push_back(), {} times: {} in {}\n",
      N1, r2, duration_cast<microseconds>(dt2));
  print("std::vector<string>, push_back(), {} times: {} in {}\n",
      N1, r3, duration_cast<microseconds>(dt3));

  return 0;
}
