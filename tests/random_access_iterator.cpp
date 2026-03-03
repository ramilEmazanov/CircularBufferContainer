#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <vector>

#include "circular_buffer.h"

template <typename T>
class CircularBufferIterTest : public testing::Test {};

using CircularBufferIterTypes = testing::Types<circular_buffer<int, false>
#ifdef RUN_EXT_TESTS
                                               ,
                                               circular_buffer<int, true>
#endif
                                               >;

TYPED_TEST_SUITE(CircularBufferIterTest, CircularBufferIterTypes);

template <class Container>
static void WrapAround(Container& cb) {
  for (int x : {7, 8, 9, 10, 11}) cb.push_back(x);
  cb.pop_front();
  cb.push_back(12);
}

TYPED_TEST(CircularBufferIterTest, IteratorTraitsAreRandomAccessAndInt) {
  using iterator = typename TypeParam::iterator;

  using category = typename std::iterator_traits<iterator>::iterator_category;
  static_assert(std::is_same_v<category, std::random_access_iterator_tag>,
                "iterator_category must be random_access_iterator_tag");

  using value_type = typename std::iterator_traits<iterator>::value_type;
  static_assert(std::is_same_v<value_type, int>, "value_type must be int");
}

TYPED_TEST(CircularBufferIterTest, Distances) {
  TypeParam cb(10);
  cb.assign({1, 3, 5, 7, 9, 11});

  auto first = cb.begin();
  auto last = cb.end();

  std::ptrdiff_t n = static_cast<std::ptrdiff_t>(cb.size());
  EXPECT_EQ(std::distance(first, last), n);
  EXPECT_EQ(last - first, n);
  EXPECT_EQ(first - last, -n);

  EXPECT_TRUE(first + n == last);
  EXPECT_TRUE(last - n == first);
}

TYPED_TEST(CircularBufferIterTest, Commutative) {
  TypeParam cb(10);
  cb.assign({1, 3, 5, 7, 9, 11});

  auto first = cb.begin();
  auto last = cb.end();

  std::ptrdiff_t n = static_cast<std::ptrdiff_t>(cb.size());
  EXPECT_TRUE(n + first == first + n);
  EXPECT_TRUE(n + first == last);

  std::ptrdiff_t k = static_cast<std::ptrdiff_t>(3);
  EXPECT_TRUE(k + first == first + k);
  EXPECT_EQ(*(k + first), *(first + k));
}

TYPED_TEST(CircularBufferIterTest, ReturnNewIterators) {
  TypeParam cb(8);
  cb.assign({100, 200, 300, 400});

  auto it = cb.begin();
  auto j = it + 2;

  EXPECT_EQ(*it, 100);
  EXPECT_EQ(*j, 300);

  auto k = j - 2;
  EXPECT_EQ(*k, 100);
}

TYPED_TEST(CircularBufferIterTest, Shifts) {
  TypeParam cb(8);
  cb.assign({9, 8, 7, 6, 5});

  auto it = cb.begin();
  it += 1;
  EXPECT_EQ(*it, 8);

  it += 3;
  EXPECT_EQ(*it, 5);

  it -= 2;
  EXPECT_EQ(*it, 7);

  it -= 0;
  EXPECT_EQ(*it, 7);
}

TYPED_TEST(CircularBufferIterTest, Brackets) {
  TypeParam cb(8);
  cb.assign({2, 4, 6, 8, 10, 12});

  auto it = cb.begin();
  EXPECT_EQ(it[0], 2);
  EXPECT_EQ(it[2], 6);
  EXPECT_EQ(it[5], 12);

  it += 3;
  EXPECT_EQ(it[0], 8);
  EXPECT_EQ(it[1], 10);
  EXPECT_EQ(it[2], 12);
}

TYPED_TEST(CircularBufferIterTest, BeginLast) {
  TypeParam cb(8);
  cb.assign({1, 2, 3});

  auto n = static_cast<std::ptrdiff_t>(cb.size());
  auto last = cb.begin() + (n - 1);
  EXPECT_EQ(*last, 3);

  auto e = cb.end();
  EXPECT_TRUE(last + 1 == e);
  EXPECT_TRUE(e - 1 == last);
}

TYPED_TEST(CircularBufferIterTest, Comparisons) {
  TypeParam cb(8);
  cb.assign({10, 20, 30, 40, 50, 60});

  auto b = cb.begin();
  auto a1 = b + 1;
  auto a3 = b + 3;
  auto e = cb.end();

  EXPECT_TRUE(b < a1);
  EXPECT_TRUE(a1 < a3);
  EXPECT_TRUE(a3 < e);

  EXPECT_TRUE(e > a3);
  EXPECT_TRUE(a3 >= a3);
  EXPECT_TRUE(a1 <= a3);

  EXPECT_FALSE(a1 < a1);
  EXPECT_FALSE(a1 > a1);
}

TYPED_TEST(CircularBufferIterTest, Arithmetic) {
  TypeParam cb(5);
  WrapAround(cb);

  auto b = cb.begin();
  auto e = cb.end();

  std::vector<int> got;
  for (auto it = b; it != e; ++it) {
    got.push_back(*it);
  }

  EXPECT_THAT(got, testing::ElementsAre(8, 9, 10, 11, 12));

  EXPECT_EQ(*(b + 0), 8);
  EXPECT_EQ(*(b + 2), 10);
  EXPECT_EQ((b + 4)[0], 12);

  EXPECT_TRUE((b + 5) == e);
  EXPECT_TRUE((e - 5) == b);
  EXPECT_EQ((e - 1)[0], 12);
}

TYPED_TEST(CircularBufferIterTest, stdAlgorithms1) {
  TypeParam cb(8);
  cb.assign({3, 1, 4, 1, 5, 9});

  std::sort(cb.begin(), cb.end());
  EXPECT_THAT(cb, testing::ElementsAre(1, 1, 3, 4, 5, 9));

  std::reverse(cb.begin(), cb.end());
  EXPECT_THAT(cb, testing::ElementsAre(9, 5, 4, 3, 1, 1));
}

TYPED_TEST(CircularBufferIterTest, stdAlgorithms2) {
  TypeParam cb(5);
  WrapAround(cb);
  cb.pop_front();
  cb.push_back(7);

  std::sort(cb.begin(), cb.end());
  EXPECT_THAT(cb, testing::ElementsAre(7, 9, 10, 11, 12));
}