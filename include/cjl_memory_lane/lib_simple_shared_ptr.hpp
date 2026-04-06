#pragma once

#include <atomic>
#include <utility>

namespace cjl::simple {
  // A simple/naive shared_ptr
  template <class T>
    class shared_ptr {
      T * p = nullptr;
      std::atomic<long long> *ctr = nullptr;
      public:
      shared_ptr() = default;
      shared_ptr(T* _p) : p{_p} {
        if (p) try {
          ctr = new std::atomic<long long>{1LL};
        } catch (...) {
          delete p;
          throw;
        }
      }

      ~shared_ptr() {
        if (ctr) {
          if (ctr->fetch_sub(1LL, std::memory_order_acq_rel) == 1LL) {
            delete p;
            delete ctr;
          }
        }
      }

      void swap(shared_ptr &other) noexcept {
        using std::swap;
        swap(p, other.p);
        swap(ctr, other.ctr);
      }

      // Copy constructor
      shared_ptr(const shared_ptr& other) : p{other.p}, ctr{other.ctr} {
        if (ctr) 
          ctr->fetch_add(1LL, std::memory_order_relaxed);
      }

      // Copy assignment
      shared_ptr& operator=(const shared_ptr& other) {
        shared_ptr{other}.swap(*this);
        return *this;
      }

      // Move constructor
      shared_ptr(shared_ptr&& other) noexcept 
        : p{std::exchange(other.p, nullptr)}
        , ctr{std::exchange(other.ctr, nullptr)} {
      }

      // Move assignment
      shared_ptr& operator=(shared_ptr&& other) noexcept {
        shared_ptr{std::move(other)}.swap(*this);
        return *this;
      }

      // Equal operator in c++20 you get != for free
      // Symmetric and rewritten candidate since c++20
      bool operator==(const shared_ptr& other) const {
        return p == other.p;
      }

      // bool operator!=(const shared_ptr& other) const {
      //   return !(p == other.p);
      // }

      T *get() noexcept { return p; }
      const T *get() const noexcept { return p; }

      T& operator*() noexcept { return *p; }
      const T& operator*() const noexcept { return *p; }

      T* operator->() noexcept { return p; }
      const T* operator->() const noexcept { return p; }
    };
} // cjl 
