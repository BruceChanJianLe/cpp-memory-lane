#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace cjl::better {
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
      : elems{static_cast<pointer>(std::malloc(n * sizeof(value_type)))},
        nelems{n}, cap{n} {
    try {
      std::uninitialized_fill(begin(), end(), init);
    } catch (...) {
      std::free(elems);
      throw;
    }
  }

  // copy ctor
  vector(const vector &other)
      : elems{static_cast<pointer>(
            std::malloc(other.size() * sizeof(value_type)))},
        // This's alright, capcity in the other are considered as artifact now
        nelems{other.size()}, cap{other.size()} {
    try {
      std::uninitialized_copy(other.begin(), other.end(), begin());
    } catch (...) {
      std::free(elems);
      throw;
    }
  }

  // move ctor
  vector(vector &&other) noexcept
      : elems{std::exchange(other.elems, nullptr)},
        nelems{std::exchange(other.nelems, 0)},
        cap{std::exchange(other.cap(), 0)} {}

  // Initializer list ctor
  vector(std::initializer_list<T> src)
      : elems{static_cast<pointer>(
            std::malloc(src.size() * sizeof(value_type)))},
        nelems{src.size()}, cap{src.size()} {
    try {
      std::uninitialized_copy(src.begin(), src.end(), begin());
    } catch (...) {
      std::free(elems);
      throw;
    }
  }

  // dtor
  ~vector() {
    std::destroy(begin(), end());
    std::free(elems);
  }

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

  // Pre condition !empty()
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
    return size() == other.size() &&
      std::equal(begin(), end(), other.begin());
  }

  void reserve(size_type new_cap) {
    // if request is the same as current
    if (new_cap <= capacity())
      return;
    auto p = static_cast<pointer>(std::malloc(new_cap * sizeof(value_type)));

    if constexpr (std::is_nothrow_move_assignable_v<T>) {
      std::uninitialized_move(begin(), end(), p);
    } else
      try {
        std::uninitialized_copy(begin(), end(), p);
      } catch (...) {
        std::free(p);
        throw;
      }
    std::destroy(begin(), end());
    std::free(elems);
    elems = p;
    cap = new_cap;
  }

  void resize(size_type new_cap) {
    if (new_cap <= capacity()) return;
    auto p = static_cast<pointer>(std::malloc(new_cap * sizeof(value_type)));

    if constexpr (std::is_nothrow_move_assignable_v<T>()) {
      std::uninitialized_move(begin(), end(), p);
    } else try {
      std::uninitialized_copy(begin(), end(), p);
    } catch (...) {
      std::free(p);
      throw;
    }
    std::uninitialized_fill(p + size(), p + capacity(), value_type{});
    std::destroy(begin(), end());
    std::free(elems);
    elems = p;
    cap = new_cap;
  }

  void push_back(const_reference val) {
    if (full())
      grow();
    std::construct_at(end(), val);
    ++nelems;
  }

  void push_back(T &&val) {
    if (full())
      grow();

    std::construct_at(end(), std::move(val));
    ++nelems;
  }

  template <class... Args> reference emplace_back(Args &&...args) {
    if (full())
      grow();

    std::construct_at(end(), std::forward<Args>(args)...);
    ++nelems;
    return back();
  }

  // Two small examples:
  // One that inserts elements at a given pos in the container
  // Another erases an element at a given pos in the container
  template <class It>
    iterator insert(const_iterator pos, It first, It last) {
      iterator pos_ = const_cast<iterator>(pos);
      // Delibrate usage of unsigned integrals
      const std::size_t remaining = capacity() - size();
      const std::size_t n = std::distance(first, last);

      // Validate if we have enough space to hold the inserted range
      if (remaining < n) {
        auto index = std::distance(begin(), pos);
        reserve(capacity() + n - remaining);
        pos_ = std::next(begin(), index);
      }

      // Validate that range does not exceed
      if (auto m = std::distance(std::next(pos_, n), end()); m > 0) {
        std::uninitialized_copy(pos_ + n, end(), end() + n - m);
        std::uninitialized_copy(pos_ + m, pos_ + n, end());
        std::copy_backward(pos_, pos_ + m, pos_ + n + m);
        std::copy(first, last, pos_);
      } else {
        std::uninitialized_copy(pos_, end(), end() + n - (n + m));
        std::uninitialized_copy(first + n + m, last, end());
        std::copy(first, first + n + m, pos_);
      }
      nelems += n;
      return pos_;
    }

  iterator erase(const_iterator pos) {
    iterator pos_ = const_cast<iterator>(pos);
    if (pos_ == end()) return pos_;
    std::copy(std::next(pos_), end(), pos_);
    *std::prev(end()) = {};
    --nelems;
    return pos_;
  }

private:
  pointer elems{};
  size_type nelems{}, cap{};
  bool full() const { return size() == capacity(); }
  void grow() { reserve(capacity() ? 2 * capacity() : 16); }
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
}; // namespace cjl::better
