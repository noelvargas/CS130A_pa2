CXX = clang++
CXXFLAGS = --std=c++11 -g -Wall -Wextra
MKDIR = mkdir

BUILDDIR = ./build
BINDIR = ./bin

.PHONY: clean directories

directories = $(BUILDDIR) $(BINDIR)
all : directories

simulator : $(BUILDDIR)/simulation.o pqueue.hpp heap.hpp
	$(CXX) $(CXXFLAGS) $< -o $(BINDIR)/$@

$(BUILDDIR)/heap_test.o : heap_test.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/pqueue_test.o : pqueue_test.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

heap : $(BUILDDIR)/heap_test.o heap.hpp
	$(CXX) $(CXXFLAGS) $< -o $(BINDIR)/$@

pqueue : $(BUILDDIR)/pqueue_test.o pqueue.hpp heap.hpp
	$(CXX) $(CXXFLAGS) $< -o $(BINDIR)/$@

directories: $(BUILDDIR) $(BINDIR)
	$(MKDIR) -p $(BUILDDIR) $(BINDIR)

clean::
	rm -rf $(BUILDDIR)/* $(BINDIR)/*
