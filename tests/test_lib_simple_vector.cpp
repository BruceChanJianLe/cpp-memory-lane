#include "cjl_memory_lane/lib_simple_vector.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <sstream>

struct Tracked {
  static inline std::atomic<int> alive{0};
  int val;
  Tracked() : val{} { ++alive; }
  explicit Tracked(int v) : val{v} { ++alive; }
  Tracked(const Tracked &o) : val{o.val} { ++alive; }
  Tracked &operator=(const Tracked &o) { val = o.val; return *this; }
  ~Tracked() { --alive; }
};

using Vec = cjl::simple::vector<int>;
using TVec = cjl::simple::vector<Tracked>;

// ── Default construction ────────────────────────────────────────────────────

TEST(SimpleVector, DefaultConstructedIsEmpty) {
  Vec v;
  EXPECT_EQ(v.size(), 0u);
  EXPECT_EQ(v.capacity(), 0u);
  EXPECT_TRUE(v.empty());
}

// ── Fill construction ───────────────────────────────────────────────────────

TEST(SimpleVector, FillConstructor) {
  Vec v(5, 7);
  EXPECT_EQ(v.size(), 5u);
  EXPECT_EQ(v.capacity(), 5u);
  for (auto x : v)
    EXPECT_EQ(x, 7);
}

// ── Initializer list ────────────────────────────────────────────────────────

TEST(SimpleVector, InitializerListConstructor) {
  Vec v{1, 2, 3};
  EXPECT_EQ(v.size(), 3u);
  EXPECT_EQ(v[0], 1);
  EXPECT_EQ(v[1], 2);
  EXPECT_EQ(v[2], 3);
}

// ── Copy semantics ──────────────────────────────────────────────────────────

TEST(SimpleVector, CopyConstructorIsDeep) {
  Vec a{1, 2, 3};
  Vec b{a};
  b[0] = 99;
  EXPECT_EQ(a[0], 1);
  EXPECT_EQ(b[0], 99);
}

TEST(SimpleVector, CopyAssignment) {
  Vec a{1, 2, 3};
  Vec b;
  b = a;
  EXPECT_EQ(b.size(), 3u);
  EXPECT_EQ(b[2], 3);
}

TEST(SimpleVector, CopyAssignmentSelf) {
  Vec v{4, 5, 6};
  v = v;
  EXPECT_EQ(v.size(), 3u);
  EXPECT_EQ(v[0], 4);
}

// ── Move semantics ──────────────────────────────────────────────────────────

TEST(SimpleVector, MoveConstructorNullsSource) {
  Vec a{10, 20, 30};
  Vec b{std::move(a)};
  EXPECT_EQ(b.size(), 3u);
  EXPECT_EQ(b[1], 20);
  EXPECT_EQ(a.size(), 0u);
  EXPECT_EQ(a.capacity(), 0u);
}

TEST(SimpleVector, MoveAssignment) {
  Vec a{7, 8, 9};
  Vec b;
  b = std::move(a);
  EXPECT_EQ(b.size(), 3u);
  EXPECT_EQ(b[0], 7);
  EXPECT_EQ(a.size(), 0u);
}

TEST(SimpleVector, MoveCtorDoesNotLeakTracked) {
  Tracked::alive = 0;
  TVec a;
  a.push_back(Tracked{1});
  a.push_back(Tracked{2});
  int before = Tracked::alive;
  {
    TVec b{std::move(a)};
    EXPECT_EQ(Tracked::alive, before);
  }
  EXPECT_EQ(Tracked::alive, 0);
}

// ── Element access ──────────────────────────────────────────────────────────

TEST(SimpleVector, FrontAndBack) {
  Vec v{1, 2, 3};
  EXPECT_EQ(v.front(), 1);
  EXPECT_EQ(v.back(), 3);
}

TEST(SimpleVector, SubscriptOperator) {
  Vec v{10, 20, 30};
  EXPECT_EQ(v[1], 20);
  v[1] = 99;
  EXPECT_EQ(v[1], 99);
}

// ── Iterators ───────────────────────────────────────────────────────────────

TEST(SimpleVector, RangeForLoop) {
  Vec v{1, 2, 3, 4, 5};
  int sum = 0;
  for (int x : v)
    sum += x;
  EXPECT_EQ(sum, 15);
}

TEST(SimpleVector, CbeginCend) {
  const Vec v{1, 2, 3};
  int sum = 0;
  for (auto it = v.cbegin(); it != v.cend(); ++it)
    sum += *it;
  EXPECT_EQ(sum, 6);
}

// ── push_back / grow ────────────────────────────────────────────────────────

TEST(SimpleVector, PushBackGrows) {
  Vec v;
  for (int i = 0; i < 20; ++i)
    v.push_back(i);
  EXPECT_EQ(v.size(), 20u);
  for (int i = 0; i < 20; ++i)
    EXPECT_EQ(v[i], i);
}

TEST(SimpleVector, PushBackMoveSemantic) {
  Tracked::alive = 0;
  TVec v;
  v.push_back(Tracked{42});
  EXPECT_EQ(v.back().val, 42);
}

// ── emplace_back ────────────────────────────────────────────────────────────

TEST(SimpleVector, EmplaceBackReturnsRef) {
  Vec v;
  auto &ref = v.emplace_back(99);
  EXPECT_EQ(ref, 99);
  EXPECT_EQ(v.back(), 99);
}

// ── resize (capacity growth, size unchanged) ────────────────────────────────

TEST(SimpleVector, ResizeGrowsCapacityOnly) {
  Vec v{1, 2, 3};
  v.resize(100);
  EXPECT_EQ(v.size(), 3u);
  EXPECT_EQ(v.capacity(), 100u);
  EXPECT_EQ(v[0], 1);
}

TEST(SimpleVector, ResizeNoOpWhenSmaller) {
  Vec v{1, 2, 3};
  v.resize(2);
  EXPECT_EQ(v.size(), 3u);
  EXPECT_EQ(v.capacity(), 3u);
}

// ── Equality ────────────────────────────────────────────────────────────────

TEST(SimpleVector, EqualityOperator) {
  Vec a{1, 2, 3};
  Vec b{1, 2, 3};
  Vec c{1, 2, 4};
  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
}

TEST(SimpleVector, EmptyVectorsAreEqual) {
  Vec a, b;
  EXPECT_EQ(a, b);
}

// ── ostream operator ────────────────────────────────────────────────────────

TEST(SimpleVector, OutputOperator) {
  Vec v{1, 2, 3};
  std::ostringstream oss;
  oss << v;
  EXPECT_EQ(oss.str(), "1,2,3");
}

TEST(SimpleVector, OutputOperatorEmpty) {
  Vec v;
  std::ostringstream oss;
  oss << v;
  EXPECT_EQ(oss.str(), "");
}

// ── Lifetime / no leak ──────────────────────────────────────────────────────

TEST(SimpleVector, DestructorReleasesAll) {
  Tracked::alive = 0;
  {
    TVec v;
    v.push_back(Tracked{1});
    v.push_back(Tracked{2});
    v.push_back(Tracked{3});
  }
  EXPECT_EQ(Tracked::alive, 0);
}
