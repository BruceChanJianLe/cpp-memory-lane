#pragma once

#include <iostream>
#include <type_traits>
#include <utility>

namespace cjl::simple {
  // Deleters
  template <class T>
  struct deleter_pointer_wrapper {
    void (*pf)(T*);
    deleter_pointer_wrapper(void (*_pf)(T*)) : pf{_pf} {}
    void operator()(T* p) {
      std::cout << "pf(p)\n";
      pf(p);
    }
  };

  template <class T>
  struct default_deleter{
    void operator()(T* p) {
      std::cout << "delete p\n";
      delete p;
    }
  };

  template <class T>
  struct default_deleter<T[]>{
    void operator()(T* p) {
      std::cout << "delete[] p\n";
      delete[] p;
    }
  };

  // Template meta programming?
  template <class T>
  struct is_deleter_function_candidate : std::false_type{};

  template <class T>
  struct is_deleter_function_candidate<void (*)(T*)> : std::true_type{};

  template <class T>
  constexpr auto is_deleter_function_candidate_v =
    is_deleter_function_candidate<T>::value;

  // A simple unique ptr
  template <class T, class D = default_deleter<T>>
  class unique_ptr : std::conditional_t<
     is_deleter_function_candidate_v<D>,
     deleter_pointer_wrapper<T>,
     D> {
    using deleter_type = std::conditional_t<
      is_deleter_function_candidate_v<D>,
      deleter_pointer_wrapper<T>,
      D>;
    T *p{nullptr};

   public:
    unique_ptr() = default;

    unique_ptr(T* _p) : p{_p} {
      std::cout << "general template\n";
    }

    unique_ptr(T* _p, void (*pf)(T*))
      : deleter_type{pf}
      , p{_p} {
        std::cout << "specialized function pointer\n";
    }

    ~unique_ptr() {
      (*static_cast<deleter_type*>(this))(p);
    }

    // Copy constructor
    unique_ptr(const unique_ptr&) = delete;
    // Copy assignment
    unique_ptr& operator=(const unique_ptr&) = delete;

    void swap(unique_ptr& other) noexcept {
      using std::swap;
      swap(p, other.p);
    }

    // Move constructor
    unique_ptr(unique_ptr&& other) noexcept :
      p{std::exchange(other.p, nullptr)} {}
    // Move assignment
    unique_ptr& operator=(unique_ptr&& other) noexcept {
      unique_ptr{std::move(other)}.swap(*this);
      return *this;
    }

    // Common operators
    bool empty() const noexcept {
      return !p;
    }

    operator bool() const noexcept { return !empty(); }

    bool operator==(const unique_ptr& other) {
      return p == other.p;
    }

    T* get() noexcept { return p; }
    const T* get() const noexcept { return p; }

    T& operator*() noexcept { return *p; }
    const T& operator*() const noexcept { return *p; }

    T* operator->() noexcept { return p; }
    const T* operator->() const noexcept { return p; }
  };

  // unique_ptr specialized for array
  template <class T, class D>
  class unique_ptr<T[], D> : std::conditional_t<
    is_deleter_function_candidate_v<D>,
    deleter_pointer_wrapper<T>,
    D>{
      using deleter_type = std::conditional_t<
        is_deleter_function_candidate_v<D>,
        deleter_pointer_wrapper<T>,
        D>;
      T* p{nullptr};
    public:
      unique_ptr() = default;

      unique_ptr(T* _p) : p{_p} {
        std::cout << "specialization for arrays\n";
      }

      unique_ptr(T* _p, void (*fp)(T*)) : deleter_type{fp}, p{_p} {
        std::cout << "specialization for arrays with function pointer\n";
      }

      ~unique_ptr() {
        (*static_cast<deleter_type*>(this))(p);
      }

      unique_ptr(const unique_ptr&) = delete;
      unique_ptr& operator=(const unique_ptr&) = delete;

      void swap(unique_ptr& other) noexcept {
        using std::swap;
        swap(p, other.p);
      }

      unique_ptr(unique_ptr&& other) noexcept
        : p{std::exchange(other.p, nullptr)}
      {}

      unique_ptr& operator=(unique_ptr&& other) noexcept {
        unique_ptr{std::move(other)}.swap(*this);
        return *this;
      }

      bool empty() const noexcept { return !p; }
      operator bool() const noexcept { return !empty(); }

      bool operator==(const unique_ptr& other) const noexcept {
        return p == other.p;
      }

      T* get() noexcept { return p; }
      const T* get() const noexcept { return p; }

      T& operator[](std::size_t n) noexcept { return p[n]; }
      const T& operator[](std::size_t n) const noexcept { return p[n]; }
  };
} // cjl::simple 
