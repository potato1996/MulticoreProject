CXX=g++
CXXFLAGS=-std=c++11 -fopenmp -O2 -Wall -I.
DEPS = Common.h \
	   CommonFilter.h \
	   helperFunctions.h \
	   murmur.h \
	   ParallelBFWNOrder.h \
	   SerialBF.h \
	   testCase.h

OBJ = test.o \
	  helperFunctions.o \
	  murmur.o \
	  ParallelBFWNOrder.o \
	  SerialBF.o \
	  testCase.o

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

amq: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS)

.PHONY: clean

clean:
	rm -f *.o amq
