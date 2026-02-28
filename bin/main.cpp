#include "circular_buffer.h"
#include <iostream>

template <class T>
void printLogic(circular_buffer<T>& circ) {
  std::cout << circ.size() << '\n';
  for (int i = 0; i < circ.size(); ++i) {
    std::cout << circ[i] << ' ';
  }
  std::cout << '\n';
}

int main() {
  circular_buffer<int> cb(5);
  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);

  cb.push_front(4);
  cb.push_front(5);

  cb.pop_back();
  cb.pop_front();

  // printLogic(cb);

  circular_buffer<int>::iterator it = cb.begin();
  // for (circular_buffer<int>::iterator it = cb.begin(); it != cb.end(); ++it) {
  //   std::cout << *it << ' ';
  // }

  for(int i = 0; i < cb.size(); ++i) {
    std::cout << *it << ' ';
    ++it;
  }

  std::cout << '\n';

  // // for(int o )
  // cb.print();
  // printLogic(cb);
}