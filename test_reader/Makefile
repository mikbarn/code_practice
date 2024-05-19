
CXXFLAGS = -I"../logger/" -Wall -Wextra


tester.exe: test_emit.o tester.cpp mab_logger.o
	$(CXX) $(CXXFLAGS) test_emit.o tester.cpp  mab_logger.o -o tester.exe

mab_logger.o: ../logger/mab_logger.cpp
	$(CXX) $(CXXFLAGS) -c ../logger/mab_logger.cpp -o mab_logger.o

test_emit.o: test_emit.cpp test_emit.h
	$(CXX) $(CXXFLAGS) -c test_emit.cpp -o test_emit.o

clean:
	rm *.o