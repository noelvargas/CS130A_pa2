#include "pqueue.hpp"
#include <iostream>

bool tiebreaker(std::string& x1, int p1, std::string& x2, int p2) {
  return x1 > x2;
}

int main() {
  PriorityQueue<std::string, tiebreaker> q;
  std::string attack = "ATTACK";
  std::string repair = "REPAIR";
  q.push(attack, 5);
  q.push(repair, 10);
  PriorityContainer<std::string> next;
  while (!q.isEmpty()) {
    next = q.pop();
    std::cout << "TIME: " << next.priority << " ACTION: " + next.content << std::endl;
  }
}
