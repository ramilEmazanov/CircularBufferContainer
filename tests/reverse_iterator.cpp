#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <vector>

#include "circular_buffer.h"

template <class Container>
void FillSimple(Container& cb) {
  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);
  cb.push_back(4);
  cb.push_back(5);
}

template <class Container>
void FillWrapAround(Container& cb) {
  for (int i = 1; i <= 5; ++i) {
    cb.push_back(i);
  }
  cb.pop_front();
  cb.push_back(6);
}

TEST(CircularBufferReverseIterTest, simpleReverse) {
  circular_buffer<int, false> cb(5);
  FillSimple(cb);

  std::vector<int> actual;
  for (auto it = cb.rbegin(); it != cb.rend(); ++it) {
    actual.push_back(*it);
  }

  EXPECT_THAT(actual, testing::ElementsAre(5, 4, 3, 2, 1));
}

TEST(CircularBufferReverseIterTest, simpleWrap) {
  circular_buffer<int, false> cb(5);
  FillWrapAround(cb);

  std::vector<int> actual;
  for (auto it = cb.rbegin(); it != cb.rend(); ++it) {
    actual.push_back(*it);
  }

  EXPECT_THAT(actual, testing::ElementsAre(6, 5, 4, 3, 2));
}

TEST(CircularBufferReverseIterTest, dereferenceIncrement) {
  circular_buffer<int, false> cb(5);
  FillSimple(cb);

  auto it = cb.rbegin();

  EXPECT_EQ(*it, 5);
  ++it;
  EXPECT_EQ(*it, 4);
  ++it;
  EXPECT_EQ(*it, 3);
}

TEST(CircularBufferReverseIterTest, dereferenceDecrement) {
  circular_buffer<int, false> cb(5);
  FillSimple(cb);

  auto it = cb.rend();
  --it;
  EXPECT_EQ(*it, 1);

  --it;
  EXPECT_EQ(*it, 2);

  --it;
  EXPECT_EQ(*it, 3);
}

TEST(CircularBufferReverseIterTest, arithmeticPlusMinus) {
  circular_buffer<int, false> cb(5);
  FillSimple(cb);

  auto it = cb.rbegin();

  EXPECT_EQ(*(it + 0), 5);
  EXPECT_EQ(*(it + 1), 4);
  EXPECT_EQ(*(it + 2), 3);
  EXPECT_EQ(*(it + 4), 1);

  it += 2;
  EXPECT_EQ(*it, 3);

  it -= 1;
  EXPECT_EQ(*it, 4);
}

TEST(CircularBufferReverseIterTest, arithmeticComparsion) {
  circular_buffer<int, false> cb(5);
  FillSimple(cb);

  auto it1 = cb.rbegin();
  auto it2 = cb.rbegin() + 2;
  auto it3 = cb.rend();

  EXPECT_TRUE(it1 < it2);
  EXPECT_TRUE(it2 > it1);
  EXPECT_TRUE(it1 <= it2);
  EXPECT_TRUE(it2 >= it1);
  EXPECT_TRUE(it3 > it1);
}

TEST(CircularBufferReverseIterTest, arithmeticCommutativity) {
  circular_buffer<int, false> cb(5);
  FillSimple(cb);

  auto first = cb.rbegin();

  EXPECT_TRUE(first + 2 == 2 + first);
  EXPECT_EQ(*(first + 2), 3);
  EXPECT_EQ(*(2 + first), 3);
}

TEST(CircularBufferReverseIterTest, operatorBrackets) {
  circular_buffer<int, false> cb(5);
  FillSimple(cb);

  auto it = cb.rbegin();

  EXPECT_EQ(it[0], 5);
  EXPECT_EQ(it[1], 4);
  EXPECT_EQ(it[2], 3);
  EXPECT_EQ(it[3], 2);
  EXPECT_EQ(it[4], 1);
}

TEST(CircularBufferReverseIterTest, decrementCorners) {
  circular_buffer<int, false> cb(5);
  FillSimple(cb);

  EXPECT_EQ(*cb.rbegin(), cb.back());

  auto it = cb.rend();
  --it;
  EXPECT_EQ(*it, cb.front());
}