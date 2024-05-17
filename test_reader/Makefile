
CXXFLAGS = -I"../logger/"


tester.exe: test_emit.o tester.cpp mab_logger.o
	$(CXX) $(CXXFLAGS) test_emit.o tester.cpp  mab_logger.o -o tester.exe

mab_logger.o: ../logger/mab_logger.cpp
	$(CXX) $(CXXFLAGS) ../logger/mab_logger.h ../logger/mab_logger.cpp -c mab_logger.o

test_emit.o: test_emit.cpp test_emit.h
	$(CXX) $(CXXFLAGS) test_emit.cpp -c test_emit.o

