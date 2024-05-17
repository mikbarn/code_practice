


test.exe: mab_logger.o log_test.o
	g++ mab_logger.o log_test.o -o test.exe


mab_logger.o: mab_logger.h mab_logger.cpp
	g++ mab_logger.h mab_logger.cpp -c mab_logger.o

log_test.o: log_test.cpp
	g++ log_test.cpp -c log_test.o