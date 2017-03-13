#include "pqueue.hpp"
#include <random>
#include <iostream>
#include <stdlib.h>


// Since subclassing seems a little overkill for the task, we will 
// just use a struct with an action property that hold the type of 
// action it represents
enum ACTION {EXECUTE_ATTACK=4, DEPLOY_ATTACK=3, EXECUTE_REPAIR=2, DEPLOY_REPAIR=1, NOTIFY=0};
struct Event {
  ACTION action;
  int source;
  int target;
};

// Sysadmin struct to simply track when its next available fix can be
// performed
struct SysAdmin {
  // ****, we're dealing with a sysadmin (https://xkcd.com/705/)
  int nextFixTime;
};

// Tiebreaker function for the MinHeap
bool tiebreaker(Event& x1, int p1, Event& x2, int p2) {
  return x1.action > x2.action;
}

// We'll communicate the end conditions with an enum. Note that the empty
// queue condition is not caught since we want the program to crash if the 
// queue is somehow emptied
enum END_CONDITIONS {QUEUE_EMPTY, NETWORK_CONQUERED, NETWORK_DEFENDED, TIMED_OUT};

/*
 * Our simulator object. Contains all of the elements of our simulation. 
 * Performs a simple fetch-execute cycle of all of the elements in the 
 * priority queue. 
 */
class Simulator {
  private:
    // Time tracking
    int t = 0;
    long long maxTime = 8640000000;

    // Simulation characteristics from the user
    int numComputers;
    int attackProbability;
    int detectProbability;

    // Actual body of the simulation state
    PriorityQueue<Event, tiebreaker> q;
    SysAdmin sysadmin;
    bool *computers;

    // Boolean for making sure we don't end the simulation before
    // the attacker has managed to successfully attack a computer
    bool hasInfected = false;

    // Fancy STL tools for random number generation
    std::mt19937 mt;
    std::uniform_int_distribution<int> prob_distribution{0, 100};
    std::uniform_int_distribution<int> comp_distribution;
    
    // Fetch-Execute cycle
    Event fetch(); 
    void process(Event& e);

    int computersInfected();

    // Helper methods for scheduling events  
    void scheduleDeployAttack(int source);
    void scheduleExecuteAttack(int source, int target);
    void scheduleDeployRepair(int target);
    void scheduleExecuteRepair(int target);
    void scheduleNotify(int source);

    // Methods for executing the events in the queue
    void processDeployAttack(Event& e);
    void processExecuteAttack(Event& e);
    void processDeployRepair(Event& e);
    void processExecuteRepair(Event& e);
    void processNotify(Event& e);

    // Methods for all of the randomness in the simulation
    bool attempt(int prob) {  return this->prob_distribution(this->mt) < prob;  }
    int randomComputer(int computer) {  
      int randComp = this->comp_distribution(this->mt);  
      return (randComp != computer) ? randComp : this->randomComputer(computer);
    }
    bool detectedByIDS(int source, int target);
  public:

    // Constructors and Deconstructors
    Simulator(int numComputers, int attackProbability, int detectProbability);
    Simulator operator=(Simulator& rhs);
    Simulator(Simulator& rhs);
    ~Simulator() {  delete[] computers;  }

    void run();
};

// Constructor
Simulator::Simulator(int numComputers, int attackProbability, int detectProbability)
  : numComputers(numComputers), attackProbability(attackProbability), detectProbability(detectProbability), comp_distribution{0, numComputers - 1} {
  this->computers = new bool[this->numComputers];
  for (int i = 0; i < this->numComputers; i++) {
    this->computers[i] = false;
  }
  auto seed = time(0);
  this->mt = std::mt19937(seed);
  this->comp_distribution = std::uniform_int_distribution<int>(0, numComputers - 1);
}

// Copy constructor
Simulator::Simulator(Simulator& s) : Simulator(s.numComputers, s.attackProbability, s.detectProbability) {
  this->t = s.t;
  this->maxTime = s.maxTime;
  this->numComputers = s.numComputers;
  this->attackProbability = s.attackProbability;
  this->detectProbability = s.detectProbability;
  this->q = s.q;
  this->sysadmin = s.sysadmin;
  delete[] this->computers;
  this->computers = new bool[this->numComputers];
  for (int i = 0; i < this->numComputers; i++) {
    this->computers[i] = s.computers[i];
  }
  this->hasInfected = s.hasInfected;
  this->mt = s.mt;
  this->prob_distribution = s.prob_distribution;
  this->comp_distribution = s.comp_distribution;
}

// Overloaded assignment operator. While admittedly not the most robust
// implementation, this program is not intended to function in an environment 
// in which it is absolutely critical that it performs
Simulator Simulator::operator=(Simulator& s) {
  delete[] this->computers;
  new (this) Simulator(s);
  return *this;
}

// Starts the simulation, runs the fetch-execute cycle, and 
// monitors the simulation for the ending condition
void Simulator::run() {
  std::cout << "STARTING SIMULATION" << std::endl;
  this->scheduleDeployAttack(-1);
  try {
    Event fetched;
    while (true) {
      fetched = this->fetch();
      this->process(fetched);
    }
  } catch (END_CONDITIONS e) {
    switch (e) {
      case NETWORK_CONQUERED:
        std::cout << "Attacker wins" << std::endl;
        break;
      case NETWORK_DEFENDED:
        std::cout << "Sysadmin wins" << std::endl
                  << std::endl << "-------------------------------------------------------------------" << std::endl << std::endl 
                  << "****, we're dealing with a sysadmin (https://xkcd.com/705/)" << std::endl
                  << std::endl << "-------------------------------------------------------------------" << std::endl << std::endl;
        break;
      case TIMED_OUT:
        std::cout << "Draw" << std::endl;
        break;
      case QUEUE_EMPTY:
        std::cerr << "The queue is empty. This is not intended. Simulation terminating." << std::endl;
        exit(1);
    }
  }
}

// The fetch part of the fetch-execute cycle
Event Simulator::fetch() {
  if (q.isEmpty()) throw QUEUE_EMPTY; 
  if (this->computersInfected() > (numComputers + 1) / 2) throw NETWORK_CONQUERED;
  if (this->computersInfected() == 0 && this->hasInfected) throw NETWORK_DEFENDED;

  auto next = q.pop();
  this->t = next.priority;
  if (this->t > maxTime) throw TIMED_OUT;
  
  return next.content;
}

// Convenient helper function for counting the infected computers
// for determining if end conditions have been reached
int Simulator::computersInfected() {
  int infected = 0;
  for (int i = 0; i < this->numComputers; i++) {
    if (this->computers[i] == true) {  infected++;  }
  }
  return infected;
}

// The execute part of the fetch-execute cycle
void Simulator::process(Event& e) {
  switch (e.action) {
    case EXECUTE_ATTACK:
      this->processExecuteAttack(e);
      break;
    case DEPLOY_ATTACK:
      this->processDeployAttack(e);
      break;
    case EXECUTE_REPAIR:
      this->processExecuteRepair(e);
      break;
    case DEPLOY_REPAIR:
      this->processDeployRepair(e);
      break;
    case NOTIFY:
      this->processNotify(e);
      break;
  }
}

// Helper methods for scheduling events in the priority queue. Also
// handle printing the output from the creation of the events.
void Simulator::scheduleNotify(int source) {
  if (source != -1) {
    Event e;
    e.action = NOTIFY;
    e.source = source;
    int t = this->t + 100;
    this->q.push(e, this->t);
    std::cout << "Notify(" << t << ", " << e.source << ")" << std::endl;
  }
}

void Simulator::scheduleDeployAttack(int source) {
  Event e;
  e.action = DEPLOY_ATTACK;
  e.source = source;
  e.target = this->randomComputer(e.source);
  int t = this->t + 1000;
  this->q.push(e, t);
  std::cout << "Deploy_Attack(" << t << ", " << e.source << ", " << e.target << ")" << std::endl;
}

void Simulator::scheduleExecuteAttack(int source, int target) {
  Event e;
  e.action = EXECUTE_ATTACK;
  e.source = source;
  e.target = target;
  int t = this->t + 100;
  this->q.push(e, this->t + 100);
  std::cout << "Execute_Attack(" << t << ", " << e.source << ", " << e.target << ")" << std::endl;
}

void Simulator::scheduleDeployRepair(int target) {
  Event e;
  e.action = DEPLOY_REPAIR;
  e.target = target;
  this->sysadmin.nextFixTime += 10000;
  int t = this->sysadmin.nextFixTime;
  this->q.push(e, t);
  std::cout << "Deploy_Repair(" << t << ", " << e.target << ")" << std::endl;
}

void Simulator::scheduleExecuteRepair(int target) {
  Event e;
  e.action = EXECUTE_REPAIR;
  e.target = target;
  int t = this->t + 100;
  this->q.push(e, this->t + 100);
  std::cout << "Execute_Repair(" << t << ", " << e.target << ")" << std::endl;
}



// The processor method to handle the execution of the events
// in the priority queue
void Simulator::processDeployAttack(Event& e) {
  if (e.source == -1 || this->computers[e.source]) {
    this->scheduleExecuteAttack(e.source, e.target);
    this->scheduleDeployAttack(e.source);
  }
}

void Simulator::processExecuteAttack(Event& e) {
  if (this->attempt(this->attackProbability)) {
    this->hasInfected = true;
    if (!this->computers[e.target]) {
      this->computers[e.target] = true;
      this->scheduleDeployAttack(e.target);
      if (this->detectedByIDS(e.source, e.target)) {
        this->scheduleNotify(e.source);
        this->scheduleNotify(e.target);
      }
    }
  }
}

void Simulator::processDeployRepair(Event &e) {
  this->scheduleExecuteRepair(e.target);
}

void Simulator::processExecuteRepair(Event &e) {
  this->computers[e.target] = false;
}

void Simulator::processNotify(Event &e) {
  this->scheduleDeployRepair(e.source);
}


// Method to determine if an attack was successfully determine by the IDS
bool Simulator::detectedByIDS(int source, int target) {
  if (source == -1) {
    return this->attempt(this->detectProbability);
  } else {
    bool sourceSide = (source >= (this->numComputers / 2));
    bool targetSide = (target >= (this->numComputers / 2));
    bool crossesIDS = (sourceSide != targetSide);
    return crossesIDS && this->attempt(this->detectProbability);
  }
}

        
int main(int argc, char** argv) {
  if (argc != 4) {
    std::cout << "Usage: simulator <num_computers> <percent_success> <percent_detect>" << std::endl;
    exit(1);
  }
  Simulator simulator(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
  simulator.run();
}
