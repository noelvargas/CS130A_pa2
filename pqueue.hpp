#ifndef PQUEUE_H
#define PQUEUE_H
#include "heap.hpp"

template<typename Contents, Tiebreaker<Contents> onTie>
class PriorityQueue {
  private:
    MinHeap<Contents, onTie> heap;
  public:
    PriorityQueue() : heap() { }
    PriorityQueue(int size) : heap(size) { }

    void push(Contents& c, long long priority) {  this->heap.push(c, priority);  }
    Contents popContent() {  return this->heap.pop().content;  }
    PriorityContainer<Contents> pop() {  return this->heap.pop();  }
    bool isEmpty() {  return this->heap.isEmpty();  }
};
#endif
