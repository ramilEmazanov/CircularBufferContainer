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
  std::vector<int> expected_elements {5, 3, 1, 0, 2, 4};
  auto it = container.begin();
  for (const auto& expected_value : expected_elements) {
    if (it == container.end() || *it != expected_value) {
      std::cout << *it << '\n';
      return false;  // Element mismatch or container size mismatch
    }
    ++it;
  }
  return true;
}

int main() {
  circular_buffer<int> cb(6);
  for (int i = 0; i < 6; ++i) {
    if (i % 2 == 0) {
      cb.push_back(i);
    } else {
      cb.push_front(i);
    }
  }

  printLogic(cb);
  // circular_buffer<int>::iterator it = cb.begin();
  // // std::cout << *it;
  // // it++;
  // // std::cout << *it;
  // // it++;
  // // std::cout << *it;
  // // it++;
  // // std::cout << *it;
  // // it++;
  // // std::cout << *it;
  // // it++;
  // // std::cout << *it;
  // // it++;
  // // std::cout << *it;




  // for (circular_buffer<int>::iterator it = cb.begin(); it != cb.end(); ++it) {
  //   std::cout << *it << ' ';
  // }
  std::cout << tester(cb);

  // circular_buffer<int> cb(5);
  // cb.push_back(1);
  // cb.push_back(2);
  // cb.push_back(3);

  // cb.push_front(4);
  // cb.push_front(5);

  // cb.pop_back();
  // cb.pop_front();

  // std::cout << '\n';

  // // for(int o )
  // cb.print();
  // printLogic(cb);
}