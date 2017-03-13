#include "heap.hpp"
#include <iostream>
#include "stdlib.h"

bool tiebreaker(int& x1, int p1, int& x2, int p2) {
  return x1 > x2;
}

void pushRange(MaxHeap<int, tiebreaker>& m, int min, int max) {
  for (int i = min; i < max+1; i++) {
    m.push(i, rand());
  }
}

void popRange(MaxHeap<int, tiebreaker>& m, int num, bool print) {
  auto popped = m.pop();
  for (int i = 0; i < num; i++) {
    if (print)
      std::cout << "CONTENT: " << popped.content << " PRIORITY: " << popped.priority << std::endl;
    popped = m.pop();
  }
}

int main() {
  srand(time(0));
  MaxHeap<int, tiebreaker> myHeap;
  int iterations = 1000;
  for (int i=0; i < iterations; i++) {
    int a = rand() % 1000;
    int b = rand() % 1000;
    int c = (a < b) ? a : b;
    int d = (a < b) ? b : a;
    pushRange(myHeap, 0, d);
    popRange(myHeap, c-1, false);
  }
  popRange(myHeap, 100, true);
}

