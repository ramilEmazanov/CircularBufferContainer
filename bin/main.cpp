#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "circular_buffer.h"

template <class T, bool U>
void printLogic(circular_buffer<T, U>& circ) {
  for (auto it = circ.begin(); it != circ.end(); ++it) {
    std::cout << *it << ' ';
  }
  std::cout << '\n';
}

template <class T>
bool tester(const T& container) {
  std::vector<int> expected_elements{5, 3, 1, 0, 2, 4};
  auto it = container.begin();
  for (const auto& expected_value : expected_elements) {
    if (it == container.end() || *it != expected_value) {
      std::cout << *it << '\n';
      return false;
    }
    ++it;
  }
  return true;
}

class NodeTag {};

class SomeObj {
 public:
  static inline int ConstructorCalled = 0;
  static inline int DestructorCalled = 0;

  SomeObj() { ++ConstructorCalled; }

  ~SomeObj() { ++DestructorCalled; }
};

template <typename T>
class TestAllocator {
 public:
  using value_type = T;
  using pointer = T*;
  using size_type = size_t;
  using is_always_equal = std::true_type;

  static inline int AllocationCount = 0;
  static inline int ElementsAllocated = 0;

  TestAllocator() = default;

  template <typename U>
  TestAllocator(const TestAllocator<U>& other) {}

  pointer allocate(size_type sz) {
    if constexpr (std::is_same_v<T, SomeObj>) {
      ++TestAllocator<SomeObj>::AllocationCount;
      TestAllocator<SomeObj>::ElementsAllocated += sz;
    } else {
      ++TestAllocator<NodeTag>::AllocationCount;
      TestAllocator<NodeTag>::ElementsAllocated += sz;
    }
    return static_cast<pointer>(std::aligned_alloc(alignof(T), sizeof(T) * sz));
  }

  void deallocate(pointer p, std::size_t) { std::free(p); }

  bool operator==(const TestAllocator& other) const { return true; }
};

template <class T>
void printRaw(circular_buffer<T>& circ) {
  for (int i = 0; i < circ.size(); ++i) {
    std::cout << circ.buff_[i] << ' ';
  }
  std::cout << '\n';
}

int main() {
  circular_buffer<int, true> cb(7);  // {3, 1, 0, 2, 4}
  for (int i = 0; i < 5; ++i) {
    if (i % 2 == 0) {
      cb.push_back(i);
    } else {
      cb.push_front(i);
    }
  }
  cb.insert(cb.begin() + 2, 2, 8);
  printLogic(cb);
}
