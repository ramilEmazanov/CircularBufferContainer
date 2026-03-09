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

TEST(CircularBufferCtorTest, EmptyBuff) {
  circular_buffer<int, false> cb(5);

  EXPECT_TRUE(cb.empty());
  EXPECT_EQ(cb.size(), 0);
  EXPECT_EQ(cb.capacity(), 5);
  EXPECT_THAT(cb, testing::ElementsAre());
}

TEST(CircularBufferCtorTest, EmptyWithAlloc) {
  typename circular_buffer<int, false>::allocator_type alloc{};

  circular_buffer<int, false> cb(alloc);

  EXPECT_TRUE(cb.empty());
  EXPECT_EQ(cb.size(), 0);
  EXPECT_THAT(cb, testing::ElementsAre());
}

TEST(CircularBufferCtorTest, ConstructorWithAlloc) {
  circular_buffer<int, false> cb(
      6, typename circular_buffer<int, false>::allocator_type());

  EXPECT_TRUE(cb.empty());
  EXPECT_EQ(cb.size(), 0);
  EXPECT_EQ(cb.capacity(), 6);

  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);

  EXPECT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferCtorTest, CtorInitializerList1) {
  circular_buffer<int, false> cb = {1, 3, 5, 7};

  EXPECT_EQ(cb.size(), 4);
  EXPECT_THAT(cb, testing::ElementsAre(1, 3, 5, 7));
}

TEST(CircularBufferCtorTest, CtorInitializerList2) {
  circular_buffer<int, false> cb = {10, 20, 30, 40, 50};

  EXPECT_THAT(cb, testing::ElementsAre(10, 20, 30, 40, 50));
}

TEST(CircularBufferCtorTest, CopyConstructorSimpleTest) {
  circular_buffer<int, false> cb(5);

  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);

  circular_buffer<int, false> copy(cb);

  EXPECT_EQ(copy.size(), cb.size());
  EXPECT_EQ(copy.capacity(), cb.capacity());
  EXPECT_THAT(copy, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferCtorTest, CopyConstructorWrap1) {
  circular_buffer<int, false> cb(5);

  WrapAround(cb);

  circular_buffer<int, false> copy(cb);

  EXPECT_EQ(copy.size(), cb.size());
  EXPECT_EQ(copy.capacity(), cb.capacity());
  EXPECT_THAT(copy, testing::ElementsAre(8, 9, 10, 11, 12));
}

TEST(CircularBufferCtorTest, CopyConstructor3) {
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

TEST(CircularBufferCtorTest, CopyConstructorWrap) {
  circular_buffer<int, false> cb(6);

  WrapAroundArbitrary(cb);

  circular_buffer<int, false> copy(cb);

  ASSERT_EQ(copy.size(), cb.size());
  EXPECT_EQ(copy.capacity(), cb.capacity());
  EXPECT_TRUE(std::equal(copy.begin(), copy.end(), cb.begin(), cb.end()));
}

TEST(CircularBufferCtorTest, CopyConstructorEmpty) {
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

TEST(CircularBufferCtorTest, CapacityNewPushes) {
  circular_buffer<int, false> cb(4);

  cb.push_back(11);
  cb.push_back(22);
  cb.push_back(33);

  EXPECT_EQ(cb.size(), 3);
  EXPECT_EQ(cb.capacity(), 4);
  EXPECT_THAT(cb, testing::ElementsAre(11, 22, 33));
}

TEST(CircularBufferCtorTest, FillConstructor1) {
  circular_buffer<int, false> cb(4, 9);

  EXPECT_EQ(cb.size(), 4);
  EXPECT_EQ(cb.capacity(), 4);
  EXPECT_THAT(cb, testing::ElementsAre(9, 9, 9, 9));
}

TEST(CircularBufferCtorTest, FillConstructor2) {
  circular_buffer<int, false> cb(1, 42);

  EXPECT_EQ(cb.size(), 1);
  EXPECT_EQ(cb.capacity(), 1);
  EXPECT_THAT(cb, testing::ElementsAre(42));
}

TEST(CircularBufferCtorTest, FillConstructor3) {
  circular_buffer<int, false> cb(0, 123);

  EXPECT_TRUE(cb.empty());
  EXPECT_EQ(cb.size(), 0);
  EXPECT_EQ(cb.capacity(), 0);
  EXPECT_THAT(cb, testing::ElementsAre());
}

TEST(CircularBufferCtorTest, FillConstructor4) {
  circular_buffer<int, false> cb(2, 123);

  EXPECT_EQ(cb.size(), 2);
  EXPECT_EQ(cb.capacity(), 2);
  EXPECT_THAT(cb, testing::ElementsAre(123, 123));
}

TEST(CircularBufferCtorTest, FillConstructorString) {
  circular_buffer<std::string, false> cb(3, std::string("abc"));

  EXPECT_EQ(cb.size(), 3);
  EXPECT_THAT(cb, testing::ElementsAre("abc", "abc", "abc"));
}

TEST(CircularBufferCtorTest, RangeConstructorFromVector) {
  std::vector<int> from{1, 2, 3, 4, 5};

  circular_buffer<int, false> cb(from.begin(), from.end());

  EXPECT_EQ(cb.size(), from.size());
  EXPECT_EQ(cb.capacity(), from.size());
  EXPECT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST(CircularBufferCtorTest, RangeConstructorFromPointers) {
  int arr[] = {10, 20, 30, 40};

  circular_buffer<int, false> cb(arr, arr + 4);

  EXPECT_EQ(cb.size(), 4);
  EXPECT_EQ(cb.capacity(), 4);
  EXPECT_THAT(cb, testing::ElementsAre(10, 20, 30, 40));
}

TEST(CircularBufferCtorTest, RangeConstructorFromEmpty) {
  std::vector<int> from;

  circular_buffer<int, false> cb(from.begin(), from.end());

  EXPECT_TRUE(cb.empty());
  EXPECT_EQ(cb.size(), 0);
  EXPECT_EQ(cb.capacity(), 0);
  EXPECT_THAT(cb, testing::ElementsAre());
}

TEST(CircularBufferCtorTest, RangeConstructor1) {
  std::vector<int> from{99};

  circular_buffer<int, false> cb(from.begin(), from.end());

  EXPECT_EQ(cb.size(), 1);
  EXPECT_EQ(cb.capacity(), 1);
  EXPECT_THAT(cb, testing::ElementsAre(99));
}

TEST(CircularBufferCtorTest, RangeConstructorWrapped) {
  circular_buffer<int, false> from(5);
  WrapAround(from);

  circular_buffer<int, false> cb(from.begin(), from.end());

  EXPECT_EQ(cb.size(), from.size());
  EXPECT_EQ(cb.capacity(), from.size());
  EXPECT_THAT(cb, testing::ElementsAre(8, 9, 10, 11, 12));
}

TEST(CircularBufferAssignTest, CopyAssignment1) {
  circular_buffer<int, false> from(5);
  from.push_back(1);
  from.push_back(2);
  from.push_back(3);

  circular_buffer<int, false> to(5);

  to = from;

  EXPECT_EQ(to.size(), from.size());
  EXPECT_EQ(to.capacity(), from.capacity());
  EXPECT_THAT(to, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferAssignTest, CopyAssignment2) {
  circular_buffer<int, false> from(5);
  WrapAround(from);

  circular_buffer<int, false> to(5);

  to = from;

  EXPECT_EQ(to.size(), from.size());
  EXPECT_EQ(to.capacity(), from.capacity());
  EXPECT_THAT(to, testing::ElementsAre(8, 9, 10, 11, 12));
}

TEST(CircularBufferAssignTest, CopyAssignment3) {
  circular_buffer<int, false> from(5);

  circular_buffer<int, false> to(5);
  to.push_back(1);
  to.push_back(2);

  to = from;

  EXPECT_TRUE(to.empty());
  EXPECT_EQ(to.size(), 0);
  EXPECT_THAT(to, testing::ElementsAre());
}

TEST(CircularBufferAssignTest, CopyAssignment4) {
  circular_buffer<int, false> from(5);
  from.push_back(1);
  from.push_back(2);
  from.push_back(3);
  from.push_back(4);

  circular_buffer<int, false> to(5);
  to.push_back(9);

  to = from;

  EXPECT_EQ(to.size(), 4);
  EXPECT_THAT(to, testing::ElementsAre(1, 2, 3, 4));
}

TEST(CircularBufferAssignTest, InitializerList1) {
  circular_buffer<int, false> cb(5);

  cb = {1, 3, 5, 7};

  EXPECT_EQ(cb.size(), 4);
  EXPECT_THAT(cb, testing::ElementsAre(1, 3, 5, 7));
}

TEST(CircularBufferAssignTest, InitializerList2) {
  circular_buffer<int, false> cb(5);

  cb.push_back(10);
  cb.push_back(20);

  cb = {7, 8, 9};

  EXPECT_EQ(cb.size(), 3);
  EXPECT_THAT(cb, testing::ElementsAre(7, 8, 9));
}