#include <circular_buffer.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

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

TEST(ExtendableBufferTest, simpleTest) {
  circular_buffer<int, true> cb(5);
  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);
  cb.push_back(4);
  cb.push_back(5);
  cb.push_back(6);
  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(ExtendableBufferTest, expand) {
  circular_buffer<int, true> cb = {1, 2, 3, 4, 5};
  cb.resize(7, 6);

  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5, 6, 6));
}

TEST(ExtendableBufferTest, shrink) {
  circular_buffer<int, true> cb = {1, 2, 3, 4, 5};
  cb.resize(3);

  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(ExtendableBufferTest, grow) {
  circular_buffer<int, true> cb = {1, 2, 3, 4, 5};
  cb.resize(7);

  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST(ExtendableBufferTest, insertValue) {
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

TEST(ExtendableBufferTest, insertInTheMiddle) {
  circular_buffer<int, true> cb(5);
  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);
  cb.push_back(4);
  cb.push_back(5);

  cb.insert(cb.begin() + 2, {6, 7, 8});
  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 6, 7, 8, 3, 4, 5));
}

TEST(ExtendableBufferTest, insertNValues) {
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

TEST(ExtendableBufferTest, insertIterators) {
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

TEST(ExtendableBufferTest, insertInitializerList) {
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

TEST(ExtendableBufferTest, simpleTestForPushBack) {
  circular_buffer<int, true> cb(5);
  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);
  cb.push_back(4);
  cb.push_back(5);
  cb.push_back(6);
  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(ExtendableBufferTest, simpleTestForPushFront) {
  circular_buffer<int, true> cb(5);
  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);
  cb.push_back(4);
  cb.push_back(5);
  cb.push_front(6);
  ASSERT_THAT(cb, testing::ElementsAre(6, 1, 2, 3, 4, 5));
}

TEST(ExtendableBufferTest, expandFullBuffer) {
  circular_buffer<int, true> cb = {1, 2, 3, 4, 5};
  cb.resize(7, 6);

  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5, 6, 6));
}

TEST(ExtendableBufferTest, expandNotFullBuffer) {
  circular_buffer<int, true> cb = {1, 2, 3, 4};
  cb.resize(7, 6);

  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 6, 6, 6));
}

TEST(ExtendableBufferTest, expandWrappedAround) {
  circular_buffer<int, true> cb(5);
  WrapAroundArbitrary(cb);
  cb.resize(7, 12);

  ASSERT_THAT(cb, testing::ElementsAre(2, 3, 4, 5, 12, 12, 12));
}

TEST(ExtendableBufferTest, expandToTheSameCapacity) {
  circular_buffer<int, true> cb(5);
  WrapAroundArbitrary(cb);
  cb.resize(5, 12);

  ASSERT_THAT(cb, testing::ElementsAre(2, 3, 4, 5, 12));
}

TEST(ExtendableBufferTest, expandToTheLessCapacity) {
  circular_buffer<int, true> cb(5);
  WrapAroundArbitrary(cb);
  cb.resize(3, 12);

  ASSERT_THAT(cb, testing::ElementsAre(2, 3, 4, 5, 12));
}

TEST(ExtendableBufferTest, insertValueFull) {
  circular_buffer<int, true> cb(5);
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

TEST(ExtendableBufferTest, insertValueNofull) {
  circular_buffer<int, true> cb(5);
  for (int i = 0; i < 4; ++i) {
    cb.push_back(i + 1);
  }
  cb.insert(cb.begin() + 2, 10);

  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 10, 3, 4));
}

TEST(ExtendableBufferTest, insertMiddleValue) {
  circular_buffer<int, true> cb = {1, 2, 3, 4};

  auto it = cb.insert(cb.begin() + 2, 10);

  EXPECT_EQ(*it, 10);
  EXPECT_EQ(it - cb.begin(), 2);
  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 10, 3, 4));
}

TEST(ExtendableBufferTest, insertBeginValue) {
  circular_buffer<int, true> cb = {1, 2, 3, 4};

  auto it = cb.insert(cb.begin(), 10);

  EXPECT_EQ(*it, 10);
  EXPECT_EQ(it, cb.begin());
  ASSERT_THAT(cb, testing::ElementsAre(10, 1, 2, 3, 4));
}

TEST(ExtendableBufferTest, insertEnd) {
  circular_buffer<int, true> cb = {1, 2, 3, 4};

  auto it = cb.insert(cb.end(), 10);

  EXPECT_EQ(*it, 10);
  EXPECT_EQ(it - cb.begin(), 4);
  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 10));
}

TEST(ExtendableBufferTest, insertCount) {
  circular_buffer<int, true> cb = {1, 2, 3, 4};

  auto it = cb.insert(cb.begin() + 1, 3, 8);

  EXPECT_EQ(*it, 8);
  EXPECT_EQ(it - cb.begin(), 1);
  ASSERT_THAT(cb, testing::ElementsAre(1, 8, 8, 8, 2, 3, 4));
}

TEST(ExtendableBufferTest, insertInitializerList1) {
  circular_buffer<int, true> cb = {1, 2, 3, 4};

  auto it = cb.insert(cb.begin() + 2, {7, 8, 9});

  EXPECT_EQ(*it, 7);
  EXPECT_EQ(it - cb.begin(), 2);
  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 7, 8, 9, 3, 4));
}

TEST(ExtendableBufferTest, insertIterators1) {
  circular_buffer<int, true> cb = {1, 2, 3, 4};
  std::vector<int> v = {10, 11, 12};

  auto it = cb.insert(cb.begin() + 1, v.begin(), v.end());

  EXPECT_EQ(*it, 10);
  EXPECT_EQ(it - cb.begin(), 1);
  ASSERT_THAT(cb, testing::ElementsAre(1, 10, 11, 12, 2, 3, 4));
}

TEST(ExtendableBufferTest, insertIteratorFromCB) {
  circular_buffer<int, true> cb = {1, 2, 3, 4};
  circular_buffer<int, true> from = {10, 11, 12};

  cb.insert(cb.begin() + 2, from.begin(), from.end());

  ASSERT_THAT(cb, testing::ElementsAre(1, 2, 10, 11, 12, 3, 4));
}