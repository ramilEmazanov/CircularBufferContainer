#include <circular_buffer.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <vector>

template <class Container>
void WrapAround(Container& cb) {
  for (int x : {7, 8, 9, 10, 11}) cb.push_back(x);
  cb.pop_front();
  cb.push_back(12);
}

template <class Container>
void WrapAroundArbitrary(Container& cb) {
  std::vector<int> nums;
  for (int i = 0; i < static_cast<int>(cb.capacity()); ++i) {
    nums.push_back(i + 1);
  }
  for (int x : nums) {
    cb.push_back(x);
  }
  cb.pop_front();
  cb.push_back(12);
}

TEST(CircularBufferCtorTest, ConstructorWithCapacityCreatesEmptyBuffer) {
  circular_buffer<int, false> cb(5);

  EXPECT_TRUE(cb.empty());
  EXPECT_EQ(cb.size(), 0);
  EXPECT_EQ(cb.capacity(), 5);
  EXPECT_THAT(cb, testing::ElementsAre());
}

TEST(CircularBufferCtorTest, ConstructorWithCapacityStoresElementsCorrectly) {
  circular_buffer<int, false> cb(5);

  cb.push_back(10);
  cb.push_back(20);
  cb.push_back(30);

  EXPECT_EQ(cb.size(), 3);
  EXPECT_EQ(cb.capacity(), 5);
  EXPECT_THAT(cb, testing::ElementsAre(10, 20, 30));
}

TEST(CircularBufferCtorTest, ConstructorWithAllocatorCreatesEmptyBuffer) {
  using alloc_t = typename circular_buffer<int, false>::allocator_type;
  alloc_t alloc{};

  circular_buffer<int, false> cb(alloc);

  EXPECT_TRUE(cb.empty());
  EXPECT_EQ(cb.size(), 0);
  EXPECT_THAT(cb, testing::ElementsAre());
}

TEST(CircularBufferCtorTest,
     ConstructorWithCapacityAndAllocatorCreatesEmptyBuffer) {
  using alloc_t = typename circular_buffer<int, false>::allocator_type;
  alloc_t alloc{};

  circular_buffer<int, false> cb(7, alloc);

  EXPECT_TRUE(cb.empty());
  EXPECT_EQ(cb.size(), 0);
  EXPECT_EQ(cb.capacity(), 7);
  EXPECT_THAT(cb, testing::ElementsAre());
}

TEST(CircularBufferCtorTest, ConstructorWithCapacityAndDefaultAllocatorWorks) {
  circular_buffer<int, false> cb(6, typename circular_buffer<int, false>::allocator_type());

  EXPECT_TRUE(cb.empty());
  EXPECT_EQ(cb.size(), 0);
  EXPECT_EQ(cb.capacity(), 6);

  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);

  EXPECT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

// TEST(CircularBufferCtorTest, DefaultConstructorCreatesEmptyBuffer) {
//   circular_buffer<int, false> cb;

//   EXPECT_TRUE(cb.empty());
//   EXPECT_EQ(cb.size(), 0);
//   EXPECT_THAT(cb, testing::ElementsAre());
// }

TEST(CircularBufferCtorTest, InitializerListConstructorStoresElementsInOrder) {
  circular_buffer<int, false> cb = {1, 3, 5, 7};

  EXPECT_EQ(cb.size(), 4);
  EXPECT_THAT(cb, testing::ElementsAre(1, 3, 5, 7));
}

TEST(CircularBufferCtorTest, InitializerListConstructorWithWrapSizedInput) {
  circular_buffer<int, false> cb = {10, 20, 30, 40, 50};

  EXPECT_THAT(cb, testing::ElementsAre(10, 20, 30, 40, 50));
}

TEST(CircularBufferCtorTest, CopyConstructorCopiesSimpleBuffer) {
  circular_buffer<int, false> cb(5);
  
  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);

  circular_buffer<int, false> copy(cb);

  EXPECT_EQ(copy.size(), cb.size());
  EXPECT_EQ(copy.capacity(), cb.capacity());
  EXPECT_THAT(copy, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferCtorTest, CopyConstructorCopiesWrappedBufferLogically) {
  circular_buffer<int, false> cb(5);

  WrapAround(cb);

  circular_buffer<int, false> copy(cb);

  EXPECT_EQ(copy.size(), cb.size());
  EXPECT_EQ(copy.capacity(), cb.capacity());
  EXPECT_THAT(copy, testing::ElementsAre(8, 9, 10, 11, 12));
}

TEST(CircularBufferCtorTest, CopyConstructorCreatesIndependentCopy) {
  circular_buffer<int, false> cb(5);
  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);

  circular_buffer<int, false> copy(cb);

  cb.pop_back();
  cb.push_back(100);

  EXPECT_THAT(copy, testing::ElementsAre(1, 2, 3));
  EXPECT_THAT(cb, testing::ElementsAre(1, 2, 100));
}

TEST(CircularBufferCtorTest, CopyConstructorCopiesArbitraryWrappedState) {
  circular_buffer<int, false> cb(6);

  WrapAroundArbitrary(cb);

  circular_buffer<int, false> copy(cb);

  ASSERT_EQ(copy.size(), cb.size());
  EXPECT_EQ(copy.capacity(), cb.capacity());
  EXPECT_TRUE(std::equal(copy.begin(), copy.end(), cb.begin(), cb.end()));
}

TEST(CircularBufferCtorTest, CopyConstructorCopiesEmptyBuffer) {
  circular_buffer<int, false> cb(5);
  circular_buffer<int, false> copy(cb);

  EXPECT_TRUE(copy.empty());
  EXPECT_EQ(copy.size(), 0);
  EXPECT_EQ(copy.capacity(), cb.capacity());
  EXPECT_THAT(copy, testing::ElementsAre());
}

TEST(CircularBufferCtorTest, CopyConstructorCopiesFullBuffer) {
  circular_buffer<int, false> cb(5);

  for (int x : {1, 2, 3, 4, 5}) {
    cb.push_back(x);
  }

  circular_buffer<int, false> copy(cb);

  EXPECT_EQ(copy.size(), 5);
  EXPECT_EQ(copy.capacity(), 5);
  EXPECT_THAT(copy, testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST(CircularBufferCtorTest, CapacityAllocatorConstructorAllowsFurtherPushes) {
  using alloc_t = typename circular_buffer<int, false>::allocator_type;
  alloc_t alloc{};

  circular_buffer<int, false> cb(4, alloc);


  cb.push_back(11);
  cb.push_back(22);
  cb.push_back(33);

  EXPECT_EQ(cb.size(), 3);
  EXPECT_EQ(cb.capacity(), 4);
  EXPECT_THAT(cb, testing::ElementsAre(11, 22, 33));
}