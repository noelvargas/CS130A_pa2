#ifndef GENERATOR_H
#define GENERATOR_H
#include <random>

using mt1337 = std::mt19937; // Because I can
class Graph {
  private:
    int** adjMatrix;
    mt1337 mt;
    std::uniform_int_distribution<int> uniform;
    std::uniform_int_distribution<int> cost;
    int getRandUniform() {
      return this->uniform(this->mt);
    }
    int getRandCost() {
      return this->cost(this->mt);
    }
  public:
    Graph(int numNodes, int seed);
    const int* const* getAdjMatrix() const { return this->adjMatrix; }
    void changeNode(int i, int j, int newValue) {
      this->adjMatrix[i][j] = this->adjMatrix[j][i] = newValue;
    }
};

Graph::Graph(int numNodes, int seed) : uniform(1, 100), cost(-120, 100) {
  this->mt.seed(seed);
  this->adjMatrix = new int*[numNodes];
  for (int i = 0; i < numNodes; i++) {
    this->adjMatrix[i] = new int[numNodes];
  }

  for (int i = 0; i < numNodes; i++) {
    for (int j = 0; j < i; j++) {
      this->adjMatrix[j][i] = adjMatrix[i][j] = getRandCost();
    }
  }

  for (int i = 0; i < numNodes; i++) {
    int max = -1337;
    int maxIndex = -1337;
    for (int j = 0; j < numNodes; j++) {
      if (this->adjMatrix[i][j] > max) {
        max = this->adjMatrix[i][j];
        maxIndex = j;
      }
      if (this->adjMatrix[i][j] <= 0) {
        this->adjMatrix[i][j] = -1337;
      }
    }
    if (max <= 0) {
      this->adjMatrix[i][maxIndex] = this->getRandUniform();
    }
  }

  for (int i = 0; i < numNodes; i++) {
    for (int j = 0; j < numNodes; j++) {
      if (this->adjMatrix[i][j] == -1337 || i == j) {
        this->adjMatrix[i][j] = 0;
      }
    }
  }
}
#endif // GENERATOR_H
