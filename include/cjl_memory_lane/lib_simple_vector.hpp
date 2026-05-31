#pragma once

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <type_traits>
#include <utility>

namespace cjl::simple {
template <class T> class vector {
public:
  using value_type = T;
  using size_type = std::size_t;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;

  size_type size() const { return nelems; }
  size_type capacity() const { return cap; }
  bool empty() const { return size() == 0; }

  using iterator = pointer;
  using const_iterator = const_pointer;

  template <class S> auto begin(this S &&self) { return self.elems; }

  template <class S> auto end(this S &&self) {
    return self.begin() + self.size();
  }

  const_iterator cbegin() const { return begin(); }
  const_iterator cend() const { return end(); }

  vector() = default;
  vector(size_type n, const_reference init)
      : elems{new value_type[n]}, nelems{n}, cap{n} {
    try {
      std::fill(begin(), end(), init);
    } catch (...) {
      delete[] elems;
      throw;
    }
  }

  // copy ctor
  vector(const vector &other)
      : elems{new value_type[other.size()]}, nelems{other.size()},
        cap{other.size()} {
    try {
      std::copy(other.begin(), other.end(), begin());
    } catch (...) {
      delete[] elems;
      throw;
    }
  }

  // move ctor
  vector(vector &&other) noexcept
      : elems{std::exchange(other.elems, nullptr)},
        nelems{std::exchange(other.nelems, 0)},
        cap{std::exchange(other.cap, 0)} {}

  // Initializer list ctor
  vector(std::initializer_list<T> src)
      : elems{new value_type[src.size()]}, nelems{src.size()}, cap{src.size()} {
    try {
      std::copy(src.begin(), src.end(), begin());
    } catch (...) {
      delete[] elems;
      throw;
    }
  }

  // dtor
  ~vector() { delete[] elems; }

  void swap(vector &other) noexcept {
    using std::swap;
    swap(elems, other.elems);
    swap(nelems, other.nelems);
    swap(cap, other.cap);
  }

  // copy assign
  vector &operator=(const vector &other) {
    vector{other}.swap(*this);
    return *this;
  }

  vector &operator=(vector &&other) noexcept {
    vector{std::move(other)}.swap(*this);
    return *this;
  }

  // c++23 (deduce this)
  template <class S> decltype(auto) operator[](this S &&self, size_type n) {
    return self.elems[n];
  }

  template <class S> decltype(auto) front(this S &&self) {
    return self.elems[0];
  }

  template <class S> decltype(auto) back(this S &&self) {
    return self.elems[self.size() - 1];
  }

  // equal operator
  // since c++20 no need to define != operator
  // when you define the == operator
  bool operator==(const vector &other) const {
    return size() == other.size() && std::equal(begin(), end(), other.begin());
  }

  void resize(size_type new_cap) {
    // if request is the same as current
    if (new_cap <= capacity())
      return;
    auto p = new value_type[new_cap];

    if constexpr (std::is_nothrow_move_assignable_v<T>) {
      std::move(begin(), end(), p);
    } else
      try {
        std::copy(begin(), end(), p);
      } catch (...) {
        delete[] p;
        throw;
      }
    delete[] elems;
    elems = p;
    cap = new_cap;
  }

  void push_back(const_reference val) {
    if (full())
      grow();
    elems[size()] = val;
    ++nelems;
  }

  void push_back(T &&val) {
    if (full())
      grow();

    elems[size()] = std::move(val);
    ++nelems;
  }

  template <class... Args> reference emplace_back(Args &&...args) {
    if (full())
      grow();

    elems[size()] = value_type(std::forward<Args>(args)...);
    ++nelems;
    return back();
  }

private:
  pointer elems{};
  size_type nelems{}, cap{};
  bool full() const { return size() == capacity(); }
  void grow() { resize(capacity() ? 2 * capacity() : 16); }
};

template <class T>
std::ostream &operator<<(std::ostream &os, const vector<T> &v) {
  if (v.empty())
    return os;
  os << v.front();
  for (auto p = std::next(v.begin()); p != v.end(); ++p) {
    os << ',' << *p;
  }
  return os;
}
}; // namespace cjl::simple
