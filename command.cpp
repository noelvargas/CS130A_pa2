#include <iostream>
#include <cstdlib>
#include <cstring>
#include <random>

#include "generator.hpp"

char *lpad(char *text, int length, char padWith) {
  char *padded = new char[length+1];
  int textLength = strlen(text);
  for (int i=0; i < length; i++) { 
    if (i >= length - textLength) {
      padded[i] = text[i - (length - textLength)];
    } else {
      padded[i] = padWith;
    }
  }
  padded[length] = '\0';
  return padded;
}

int main(int argc, char **argv) {
  int seed;
  if (argc == 2) {
    seed = (std::random_device())();
  } else if (argc == 3) {
    seed = atoi(argv[2]);
  } else {
    std::cout << "Usage: generate <num_nodes> [<seed>]" << std::endl;
    exit(1);
  }

  int numNodes = atoi(argv[1]);
  Graph g(numNodes, seed);
  const int * const* adjMatrix = g.getAdjMatrix();
  for (int i = 0; i < numNodes; i++) {
    for (int j = 0; j < numNodes; j++) {
      char element[4];
      sprintf(element, "%d", adjMatrix[i][j]);
      char *padded = lpad(element, 3, ' ');
      std::cout << padded << " "; 
      delete[] padded;
    }
    std::cout << std::endl;
  }

  return 0;
}

