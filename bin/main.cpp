#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "circular_buffer.h"

template <class T>
void printLogic(circular_buffer<T>& circ) {
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

template <class T>
void printRaw(circular_buffer<T>& circ) {
  for (int i = 0; i < circ.size(); ++i) {
    std::cout << circ.buff_[i] << ' ';
  }
  std::cout << '\n';
}

int main() {
  circular_buffer<int> cb = {5, 4, 3, 2, 1};
  printLogic(cb);

  cb.assign({1, 2, 3, 4, 5});


  // circular_buffer<std::string> cb(3);
  // cb.push_back("aaa");
  // cb.push_back("bbb");
  // cb.push_back("ccc");
  // printLogic(cb);
  // cb.assign(5, 7);
  // for (int i = 0; i < 5; ++i) {
  //   if (i % 2 == 0) {
  //     cb.push_back(i);
  //   } else {
  //     cb.push_front(i);
  //   }
  //   printLogic(cb);
  // }
}
