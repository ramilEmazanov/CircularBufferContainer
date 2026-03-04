#include <circular_buffer.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

template <typename T>
class CircularBufferIntTestExtended : public testing::Test {};

using CircularBufferIntTypes = testing::Types<circular_buffer<int, false>
#ifdef RUN_EXT_TESTS
                                              ,
                                              circular_buffer<int, true>
#endif
                                              >;
TYPED_TEST_SUITE(CircularBufferIntTestExtended, CircularBufferIntTypes);

template <class Container>
void WrapAround(Container& cb) {
  for (int x : {7, 8, 9, 10, 11}) cb.push_back(x);
  cb.pop_front();
  cb.push_back(12);
}

template <class Container>
void WrapAroundArbitrary(Container& cb) {
  std::vector<int> nums;
  for (int i = 0; i < cb.capacity(); ++i) {
    nums.push_back(i + 1);
  }
  for (int x : nums) {
    cb.push_back(x);
  }
  cb.pop_front();
  cb.push_back(12);
}

TEST(CircularBufferIntTestExtended, simpleTest) {
  circular_buffer<int, true> cb(5);
  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);
  cb.push_back(4);
  cb.push_back(5);
  cb.push_back(6);
  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(CircularBufferIntTestExtended, insertInTheMiddle) {
  circular_buffer<int, true> cb(5);
  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);
  cb.push_back(4);
  cb.push_back(5);

  cb.insert(cb.begin() + 2, {6, 7, 8});
  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 6, 7, 8, 3, 4, 5));
}

TYPED_TEST(CircularBufferIntTestExtended, expandFullBuffer) {
  circular_buffer<int, true> cb = {1, 2, 3, 4, 5};
  cb.resize(7, 6);

  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5, 6, 6));
}

TYPED_TEST(CircularBufferIntTestExtended, expandNotFullBuffer) {
  circular_buffer<int, true> cb = {1, 2, 3, 4, 5};
  cb.resize(7, 6);

  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5, 6, 6));
}

TYPED_TEST(CircularBufferIntTestExtended, expandWrappedAround) {
  circular_buffer<int, true> cb(5);
  WrapAroundArbitrary(cb);
  cb.resize(7, 12);

  ASSERT_THAT(cb, testing::ElementsAre(2, 3, 4, 5, 12, 12, 12));
}

TYPED_TEST(CircularBufferIntTestExtended, shrink) {
  circular_buffer<int, true> cb = {1, 2, 3, 4, 5};
  cb.resize(3);
  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TYPED_TEST(CircularBufferIntTestExtended, insertValue) {
  circular_buffer<int, true> cb(5);  // {3, 1, 0, 2, 4}
  for (int i = 0; i < 5; ++i) {
    if (i % 2 == 0) {
      cb.push_back(i);
    } else {
      cb.push_front(i);
    }
  }
  cb.insert(cb.begin() + 2, 10);

  ASSERT_THAT(cb, testing::ElementsAre(3, 1, 10, 0, 2, 4));
}

TYPED_TEST(CircularBufferIntTestExtended, insertNValues) {
  circular_buffer<int, true> cb(7);  // {3, 1, 0, 2, 4}
  for (int i = 0; i < 5; ++i) {
    if (i % 2 == 0) {
      cb.push_back(i);
    } else {
      cb.push_front(i);
    }
  }
  cb.insert(cb.begin() + 2, 2, 8);

  ASSERT_THAT(cb, testing::ElementsAre(3, 1, 8, 8, 0, 2, 4));
}

TYPED_TEST(CircularBufferIntTestExtended, insertIterators) {
  circular_buffer<int, true> cb(8);  // {3, 1, 0, 2, 4}
  std::vector<int> v = {1, 2, 3, 4, 5};
  for (int i = 0; i < 5; ++i) {
    if (i % 2 == 0) {
      cb.push_back(i);
    } else {
      cb.push_front(i);
    }
  }
  cb.insert(cb.begin() + 2, v.begin() + 1, v.end() - 1);

  ASSERT_THAT(cb, testing::ElementsAre(3, 1, 2, 3, 4, 0, 2, 4));
}

TYPED_TEST(CircularBufferIntTestExtended, insertInitializerList) {
  circular_buffer<int, true> cb(8);  // {3, 1, 0, 2, 4}
  for (int i = 0; i < 5; ++i) {
    if (i % 2 == 0) {
      cb.push_back(i);
    } else {
      cb.push_front(i);
    }
  }
  cb.insert(cb.begin() + 2, {2, 3, 4});

  ASSERT_THAT(cb, testing::ElementsAre(3, 1, 2, 3, 4, 0, 2, 4));
}
