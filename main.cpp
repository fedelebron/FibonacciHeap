#include "fibonacci_heap.hpp"

#include <iostream>
#include <random>
using fibonacci_heap::heap;

int main() {
  heap<int> h;
  std::random_device rd;
  std::default_random_engine gen(rd());
  std::uniform_int_distribution<int> d(0, 100);
  for (int i = 0; i < 500000; ++i) {
    auto x = d(gen);
    auto it = h.insert(x, x);
    h.decreaseKey(it, x - 1);
  }
  
  std::vector<int> v;
  while (!h.empty()) v.push_back(h.extractMin());
  for (int i = 0; i < v.size() - 1; ++ i) {
    if (v[i] > v[i + 1]) {
      std::cerr << "Whoops.\n";
    }
  }
}
