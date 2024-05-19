CXX = g++
CXXFLAGS = -I"../logger/" -I"../test_reader"


test_knapsack.exe: test_knapsack.cpp test_emit.o knapsack.o mab_logger.o
	$(CXX) $(CXXFLAGS) test_knapsack.cpp test_emit.o  mab_logger.o -o test_knapsack.exe

knapsack.o: knapsack.cpp
	$(CXX) $(CXXFLAGS) -c knapsack.cpp -o knapsack.o


mab_logger.o: ../logger/mab_logger.cpp
	$(CXX) $(CXXFLAGS) -c ../logger/mab_logger.cpp -o mab_logger.o

test_emit.o: ../test_reader/test_emit.cpp ../test_reader/test_emit.h
	$(CXX) $(CXXFLAGS) -c ../test_reader/test_emit.cpp -o test_emit.o

