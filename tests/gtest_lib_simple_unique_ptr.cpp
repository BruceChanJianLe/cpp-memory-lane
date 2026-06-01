#include "cjl_memory_lane/lib_simple_unique_ptr.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <type_traits>

struct Tracked {
  static inline std::atomic<int> alive{0};
  int val;
  explicit Tracked(int v) : val{v} { ++alive; }
  ~Tracked() { --alive; }
};

using Ptr = cjl::simple::unique_ptr<Tracked>;

// ── Ownership and lifetime ──────────────────────────────────────────────────

TEST(UniquePtr, DefaultConstructedIsNull) {
  Ptr p;
  EXPECT_EQ(p.get(), nullptr);
  EXPECT_TRUE(p.empty());
  EXPECT_FALSE(static_cast<bool>(p));
}

TEST(UniquePtr, SingleOwnerDestroysOnScopeExit) {
  Tracked::alive = 0;
  {
    Ptr p{new Tracked{42}};
    EXPECT_EQ(p->val, 42);
    EXPECT_EQ(Tracked::alive, 1);
  }
  EXPECT_EQ(Tracked::alive, 0);
}

TEST(UniquePtr, NullPtrDoesNotCrashOnDestruction) {
  Ptr p{nullptr};
  // destructor must call deleter(nullptr); default_deleter calls delete nullptr (safe)
}

// ── Move semantics ──────────────────────────────────────────────────────────

TEST(UniquePtr, MoveConstructorNullsSource) {
  Tracked::alive = 0;
  Ptr p1{new Tracked{7}};
  Tracked* raw = p1.get();
  Ptr p2{std::move(p1)};
  EXPECT_EQ(p1.get(), nullptr);
  EXPECT_EQ(p2.get(), raw);
  EXPECT_EQ(Tracked::alive, 1);
}

TEST(UniquePtr, MoveAssignmentNullsSource) {
  Tracked::alive = 0;
  Ptr p1{new Tracked{9}};
  Ptr p2;
  p2 = std::move(p1);
  EXPECT_EQ(p1.get(), nullptr);
  EXPECT_EQ(p2->val, 9);
  EXPECT_EQ(Tracked::alive, 1);
}

TEST(UniquePtr, MoveAssignmentDropsPreviousValue) {
  Tracked::alive = 0;
  Ptr p1{new Tracked{1}};
  Ptr p2{new Tracked{2}};
  p2 = std::move(p1);        // p2's old Tracked{2} must be destroyed
  EXPECT_EQ(Tracked::alive, 1);
  EXPECT_EQ(p2->val, 1);
}

// ── Copy is deleted ─────────────────────────────────────────────────────────

TEST(UniquePtr, CopyIsDeleted) {
  static_assert(!std::is_copy_constructible_v<Ptr>);
  static_assert(!std::is_copy_assignable_v<Ptr>);
}

// ── Accessors ───────────────────────────────────────────────────────────────

TEST(UniquePtr, DerefOperators) {
  Ptr p{new Tracked{55}};
  EXPECT_EQ((*p).val, 55);
  EXPECT_EQ(p->val, 55);
}

TEST(UniquePtr, OperatorBool) {
  Ptr empty;
  Ptr full{new Tracked{1}};
  EXPECT_FALSE(static_cast<bool>(empty));
  EXPECT_TRUE(static_cast<bool>(full));
}

TEST(UniquePtr, Equality) {
  Ptr null1, null2;
  EXPECT_EQ(null1, null2);           // both null → equal
  Ptr p{new Tracked{1}};
  EXPECT_NE(p, null1);               // non-null vs null → not equal
}

// ── Array specialisation ────────────────────────────────────────────────────

TEST(UniquePtrArray, DefaultConstructedIsNull) {
  cjl::simple::unique_ptr<Tracked[]> p;
  EXPECT_EQ(p.get(), nullptr);
}

TEST(UniquePtrArray, OwnsAndDestroysArray) {
  Tracked::alive = 0;
  {
    auto* arr = new Tracked[3]{Tracked{1}, Tracked{2}, Tracked{3}};
    cjl::simple::unique_ptr<Tracked[]> p{arr};
    EXPECT_EQ(Tracked::alive, 3);
    EXPECT_EQ(p[0].val, 1);
    EXPECT_EQ(p[1].val, 2);
    EXPECT_EQ(p[2].val, 3);
  }
  EXPECT_EQ(Tracked::alive, 0);
}

TEST(UniquePtrArray, MoveConstructorNullsSource) {
  Tracked::alive = 0;
  cjl::simple::unique_ptr<Tracked[]> p1{new Tracked[2]{Tracked{10}, Tracked{20}}};
  auto* raw = p1.get();
  cjl::simple::unique_ptr<Tracked[]> p2{std::move(p1)};
  EXPECT_EQ(p1.get(), nullptr);
  EXPECT_EQ(p2.get(), raw);
  EXPECT_EQ(Tracked::alive, 2);
}

TEST(UniquePtrArray, MoveAssignmentNullsSource) {
  Tracked::alive = 0;
  cjl::simple::unique_ptr<Tracked[]> p1{new Tracked[2]{Tracked{1}, Tracked{2}}};
  cjl::simple::unique_ptr<Tracked[]> p2;
  p2 = std::move(p1);
  EXPECT_EQ(p1.get(), nullptr);
  EXPECT_EQ(p2[0].val, 1);
  EXPECT_EQ(Tracked::alive, 2);
}

// ── Custom deleter (function pointer) ───────────────────────────────────────

TEST(UniquePtrCustomDeleter, FunctionPointerDeleterIsCalled) {
  static bool called = false;
  void (*deleter)(Tracked*) = [](Tracked* p) { called = true; delete p; };
  {
    cjl::simple::unique_ptr<Tracked, void(*)(Tracked*)> p{new Tracked{1}, deleter};
  }
  EXPECT_TRUE(called);
}
