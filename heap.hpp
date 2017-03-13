#ifndef HEAP_H
#define HEAP_H

/*
 * Container for the heap. Has a field for the content and a tag to support 
 * the priority within the heap. Has overloaded comparison operators to make 
 * writing tiebreaker functions more convenient
 */
template<typename Content>
class PriorityContainer {
  public: 
    PriorityContainer() : priority(0) { }
    PriorityContainer(Content content, long long priority) : content(content), priority(priority) { }
    Content content;
    long long priority;
    
    bool operator==(PriorityContainer<Content> const& rhs) {  return this->priority == rhs.priority;  }
    bool operator<(PriorityContainer<Content> const& rhs) {  return this->priority < rhs.priority;  }
    bool operator>(PriorityContainer<Content> const& rhs) {  return this->priority > rhs.priority;  }

    bool operator!=(PriorityContainer<Content> const& rhs) {  return !(*this == rhs);  }
    bool operator<=(PriorityContainer<Content> const& rhs) {  return (*this == rhs) || (*this < rhs);  }
    bool operator>=(PriorityContainer<Content> const& rhs) {  return (*this == rhs) || (*this > rhs);  }
};

// Since not much info is needed from the error messages, it is easiest 
// to just return an error code
enum HeapErrorCodes {OVERRIDE, NO_ELEMENT, NO_CHILD}; 


/* 
 * The main heap object. Contains a dynamically allocated array that holds
 * all of the PriorityContainer objects. Keeps track of the elements in the 
 * array by a count of the occupied cells and the total number of cells, 
 * allowing it to automatically resize itself when necessary. 
 *
 * NOTE: this class should be subclassed to be used. The comparison function
 * moreTop is meant to be overriden.
 */
template<typename NodeContents>
class Heap {
  private:
    int size;
    int occupied;
    PriorityContainer<NodeContents> *contents;

    int getLastIndex() {  return this->occupied - 1;  }
    int getOpenIndex() {  return this->occupied;  }

    void place(PriorityContainer<NodeContents> x, int index);

    PriorityContainer<NodeContents> grab(int index);

    virtual bool moreTop(PriorityContainer<NodeContents>& x1, 
                         PriorityContainer<NodeContents>& x2) {  return true;  }
    bool compare(int index1, int index2) {
      auto node1 = this->grab(index1);
      auto node2 = this->grab(index2);
      return this->moreTop(node1, node2);
    }
    int returnTopper(int index1, int index2);

    void percolateDown(int index);
    void percolateUp(int index);

    int getRightChildIndex(int index);
    int getLeftChildIndex(int index);
    int getChildIndex(int index, bool isLeft);
    PriorityContainer<NodeContents> getRightChild(int index);
    PriorityContainer<NodeContents> getLeftChild(int index);
    PriorityContainer<NodeContents> getChild(int index, bool isLeft);
    int getParentIndex(int index);
    PriorityContainer<NodeContents> getParent(int index);
    PriorityContainer<NodeContents> getNode(int index);
    bool hasNode(int index);
  public:
    Heap();
    Heap(int initialSize);
    Heap(Heap& h);
    Heap(Heap<NodeContents>&& other);
    Heap<NodeContents> operator=(Heap<NodeContents>& h);
    ~Heap();

    void push(PriorityContainer<NodeContents> x);
    void push(NodeContents x, int priority) {
      this->push(PriorityContainer<NodeContents>(x, priority));
    }
    PriorityContainer<NodeContents> pop();

    // Since I make no assumptions about the comparison function 
    // except that it gives a legitimate location in the heap for
    // the object, it is necessary to try both percolateUp'ing and
    // percolateDown'ing
    void incKey(int index, int val) {
      this->contents[index].priority += val;
      this->percolateUp(index);
      this->percolateDown(index);
    }
    
    void decKey(int index, int val) {
      this->contents[index].priority -= val;
      this->percolateUp(index);
      this->percolateDown(index);
    }

    bool isEmpty() {  return this->occupied == 0;  }
};

// Create an alias for the type of Tiebreaker function pointers
template<typename Content>
using Tiebreaker = bool (*)(Content& c1, int p1, Content& c2, int p2);

/*
 * The actual classes that are mean to be used. Min and Max heap are both
 * templated on a tiebreaker function, which compares two objects in the 
 * case that they both have the same priority. They are templated over this
 * function because the changing of this function utterly changes the ordering
 * of the heap and therefore the type of the heap itself
 */
template<typename NodeContents, Tiebreaker<NodeContents> onTie>
class MinHeap : public Heap<NodeContents> {
  private:
    bool moreTop(PriorityContainer<NodeContents>& x1, 
                 PriorityContainer<NodeContents>& x2) { 
      if (x1 == x2) {
        return onTie(x1.content, x1.priority, x2.content, x2.priority);
      } else {
        return x1 < x2;
      }
    }
};

template<typename NodeContents, Tiebreaker<NodeContents> onTie>
class MaxHeap : public Heap<NodeContents> {
  private:
    bool moreTop(PriorityContainer<NodeContents>& x1, 
                 PriorityContainer<NodeContents>& x2) { 
      if (x1 == x2) {
        return onTie(x1.content, x1.priority, x2.content, x2.priority);
      } else {
        return x1 > x2;
      }
    }
};

// Constructors and Destructors
template<typename NodeContents>
Heap<NodeContents>::Heap(int initialSize) {
  this->contents = new PriorityContainer<NodeContents>[initialSize];
  this->size = initialSize;
  this->occupied = 0;
}

// Guess a good starting size for the user
template<typename NodeContents>
Heap<NodeContents>::Heap() : Heap(20) { }

// Copy constructor
template<typename NodeContents>
Heap<NodeContents>::Heap(Heap<NodeContents>& rhs) : Heap() {
  this->contents = new PriorityContainer<NodeContents>[rhs.size];
  for (int i = 0; i < rhs.occupied; i++) {
    this->contents[i] = rhs.contents[i];
  }
  this->occupied = rhs.occupied;
  this->size = rhs.size;
}

// Overloaded assignment operator. While admittedly not the most robust
// implementation, this program is not intended to function in an environment 
// in which it is absolutely critical that it performs
template<typename NodeContents>
Heap<NodeContents> Heap<NodeContents>::operator=(Heap<NodeContents>& rhs) {
  delete[] this->contents;
  new (this) Heap(rhs);
  return *this;
}

template<typename NodeContents>
Heap<NodeContents>::~Heap() {
  delete[] this->contents;
}


/* push: 
 * Checks the size of the current heap and resizes the dynamic array in 
 * memory, copying the data between the old and new arrays. To make sure we're
 * not constantly performing the relatively expensive operation of resizing the
 * heap, we double the size each time we resize. 
 * For the actual operation, we simply place the new item in the bottom-most index
 * and then percolate it upwards until it finds its correct position. 
 */
template<typename NodeContents>
void Heap<NodeContents>::push(PriorityContainer<NodeContents> x) {
  if (this->occupied >= this->size) {
    int newSize = this->occupied * 2;
    PriorityContainer<NodeContents> *newContents = new PriorityContainer<NodeContents>[newSize];
    for (int i = 0; i < this->occupied; i++) {
      newContents[i] = this->contents[i];
    }
    delete[] this->contents;
    this->contents = newContents;
    this->size = newSize;
  }

  this->place(x, this->getOpenIndex());
  this->occupied++;
  this->percolateUp(this->getLastIndex());
}

/* pop:
 * Gets the node to return, and then addresses the edge case of us having removed
 * the last node in the heap. Grabs the bottom-most element in the heap, places it
 * in the top where we just removed an element, and then percolates it downward. 
 * Eventually returns the value we popped off the heap.
 */
template<typename NodeContents>
PriorityContainer<NodeContents> Heap<NodeContents>::pop() {
  PriorityContainer<NodeContents> toReturn = this->grab(0);
  if (this->getLastIndex() == 0) {
    this->occupied--;
    return toReturn;
  }
  auto lastNode = this->grab(this->getLastIndex());
  this->occupied--;
  this->place(lastNode, 0);
  this->percolateDown(0);
  return toReturn;
}
 

// Standard percolation methods for helping elements to find their correct
// place in the heap
template<typename NodeContents>
void Heap<NodeContents>::percolateDown(int index) {
  int leftChildIndex = this->getLeftChildIndex(index);
  int rightChildIndex = this->getRightChildIndex(index);

  try {
    int compareAgainst = this->returnTopper(leftChildIndex, rightChildIndex);
    if (!this->compare(index, compareAgainst)) {
      auto tempNode = this->grab(compareAgainst);
      this->place(this->grab(index), compareAgainst);
      this->place(tempNode, index);
      return this->percolateDown(compareAgainst);
    }
  } catch (const HeapErrorCodes&) {
    return;
  }
}

template<typename NodeContents>
void Heap<NodeContents>::percolateUp(int index) {
  int parentIndex = this->getParentIndex(index);
  if (this->hasNode(parentIndex)) {
    auto currentNode = this->grab(index);
    auto parentNode = this->grab(parentIndex);
    if (this->moreTop(currentNode, parentNode)) {
      this->place(currentNode, parentIndex);
      this->place(parentNode, index);
      this->percolateUp(parentIndex);
    }
  }
}

// Helpful methods for comparing two indices in the heap. Returns the 
// index of the more upper element
template<typename NodeContents>
int Heap<NodeContents>::returnTopper(int index1, int index2) {
  if (!this->hasNode(index1) && !this->hasNode(index2)) {
    throw NO_CHILD;  
  } else if (!this->hasNode(index1)) {
    return index2;
  } else if (!this->hasNode(index2)) {
    return index1;
  } else {
    auto node1 = this->grab(index1);
    auto node2 = this->grab(index2);
    return (this->moreTop(node1, node2)) ? index1 : index2;
  }
}



// Check to make sure we don't accidentally index outside of our array
template<typename NodeContents>
bool Heap<NodeContents>::hasNode(int index) {
  return (index >= 0) && (index < this->occupied);
}

// Helper methods for getting children and parents of indices in the heap
template<typename NodeContents>
int Heap<NodeContents>::getLeftChildIndex(int index) {
  return this->getChildIndex(index, true);
}

template<typename NodeContents>
int Heap<NodeContents>::getRightChildIndex(int index) {
  return this->getChildIndex(index, false);
}

template<typename NodeContents>
int Heap<NodeContents>::getChildIndex(int index, bool isLeft) {
  if (isLeft) {
    return 2 * index + 1;
  } else {
    return 2 * index + 2;
  }
}

template<typename NodeContents>
int Heap<NodeContents>::getParentIndex(int index) {
  if (index == 0) {  return -1;  }
  return (index - 1) / 2;
}

// Element moving. Provide some checks and abstract away the array details
// so that more checking can be easily added if necessary.
template<typename NodeContents>
void Heap<NodeContents>::place(PriorityContainer<NodeContents> x, int index) {
  this->contents[index] = x;
}

template<typename NodeContents>
PriorityContainer<NodeContents> Heap<NodeContents>::grab(int index) {
  if (!this->hasNode(index)) {  throw NO_ELEMENT;  }
  return this->contents[index];
}

#endif
