#include <exception>
#include <iostream>
#include <vector>

#include "circular_buffer.h"

template <class T>
void printLogic(circular_buffer<T>& circ) {
  std::cout << circ.size() << '\n';
  for (int i = 0; i < circ.size(); ++i) {
    std::cout << circ[i] << ' ';
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

int main() {
  circular_buffer<int> cb(5);
  circular_buffer<int> listed{1, 2, 3, 4, 5};

  // listed.erase(listed.cbegin() + 2);
  // std::cout << *(listed.cbegin() + 2) << '\n';

  for (int i = 0; i < 20; ++i) {
    if (i % 2 == 0) {
      cb.push_back(i);
    } else {
      cb.push_front(i);
    }
    printLogic(listed);
  }
}