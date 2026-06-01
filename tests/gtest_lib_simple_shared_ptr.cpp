#include "cjl_memory_lane/lib_simple_shared_ptr.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

// Tracks live instances via a static counter so we can assert destruction.
struct Tracked {
  static inline std::atomic<int> alive{0};
  int val;
  explicit Tracked(int v) : val{v} { ++alive; }
  ~Tracked() { --alive; }
};

using Ptr = cjl::simple::shared_ptr<Tracked>;

TEST(SharedPtr, DefaultConstructedIsNull) {
  Ptr p;
  EXPECT_EQ(p.get(), nullptr);
}

TEST(SharedPtr, SingleOwnerDestroysOnScopeExit) {
  Tracked::alive = 0;
  {
    Ptr p{new Tracked{42}};
    EXPECT_EQ(p->val, 42);
    EXPECT_EQ(Tracked::alive, 1);
  }
  EXPECT_EQ(Tracked::alive, 0);
}

TEST(SharedPtr, CopySharesOwnership) {
  Tracked::alive = 0;
  {
    Ptr p1{new Tracked{1}};
    {
      Ptr p2 = p1;
      EXPECT_EQ(p1.get(), p2.get());
      EXPECT_EQ(Tracked::alive, 1); // one object, two owners
    }
    EXPECT_EQ(Tracked::alive, 1); // p2 gone, p1 still holds it
  }
  EXPECT_EQ(Tracked::alive, 0);
}

TEST(SharedPtr, CopyAssignmentSharesOwnership) {
  Tracked::alive = 0;
  Ptr p1{new Tracked{10}};
  Ptr p2;
  p2 = p1;
  EXPECT_EQ(p1.get(), p2.get());
  EXPECT_EQ(Tracked::alive, 1);
  p1 = Ptr{}; // drop p1
  EXPECT_EQ(Tracked::alive, 1); // p2 still holds it
}

TEST(SharedPtr, MoveConstructorNullsSource) {
  Tracked::alive = 0;
  Ptr p1{new Tracked{7}};
  Tracked* raw = p1.get();
  Ptr p2 = std::move(p1);
  EXPECT_EQ(p1.get(), nullptr);
  EXPECT_EQ(p2.get(), raw);
  EXPECT_EQ(Tracked::alive, 1);
}

TEST(SharedPtr, MoveAssignmentNullsSource) {
  Tracked::alive = 0;
  Ptr p1{new Tracked{9}};
  Ptr p2;
  p2 = std::move(p1);
  EXPECT_EQ(p1.get(), nullptr);
  EXPECT_EQ(p2->val, 9);
  EXPECT_EQ(Tracked::alive, 1);
}

TEST(SharedPtr, EqualityComparesPointers) {
  Ptr p1{new Tracked{1}};
  Ptr p2 = p1;
  Ptr p3{new Tracked{2}};
  EXPECT_EQ(p1 == p2, true);
  EXPECT_NE(p1 != p3, false);
}

TEST(SharedPtr, DerefOperators) {
  Ptr p{new Tracked{55}};
  EXPECT_EQ((*p).val, 55);
  EXPECT_EQ(p->val, 55);
}

// Many threads each copy the shared_ptr, read through it, then drop their copy.
// The object must survive until all threads are done, then be destroyed exactly once.
TEST(SharedPtr, ThreadedCopyAndDrop) {
  Tracked::alive = 0;
  constexpr int N = 8;
  {
    Ptr root{new Tracked{99}};
    std::vector<std::thread> threads;
    threads.reserve(N);
    for (int i = 0; i < N; ++i) {
      threads.emplace_back([root] {
        Ptr local = root;
        EXPECT_EQ(local->val, 99);
      });
    }
    for (auto& t : threads) t.join();
    EXPECT_EQ(Tracked::alive, 1); // root still alive
  }
  EXPECT_EQ(Tracked::alive, 0);
}
