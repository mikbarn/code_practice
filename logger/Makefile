


test.exe: mab_logger.o log_test.o
	g++ mab_logger.o log_test.o -o test.exe


mab_logger.o: mab_logger.h mab_logger.cpp
	g++ -c mab_logger.cpp 

log_test.o: log_test.cpp
	g++ -c log_test.cpp 

clean:
	rm *.o