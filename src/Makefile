CXX=g++
CXXFLAGS=-std=c++11 -fopenmp -O2 -Wall -I.
DEPS = Common.h \
	   CommonFilter.h \
	   helperFunctions.h \
	   helperFunctionsQF.h \
	   murmur.h \
	   ParallelBFWNOrder.h \
	   SerialBF.h \
	   ParallelQFWNOrder.h \
	   SerialQF.h \
	   testCase.h

OBJ = test.o \
	  helperFunctions.o \
	  helperFunctionsQF.o \
	  murmur.o \
	  ParallelBFWNOrder.o \
	  SerialBF.o \
	  ParallelQFWNOrder.o \
	  SerialQF.o \
	  testCase.o

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

amq: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS)

.PHONY: clean

clean:
	rm -f *.o amq
