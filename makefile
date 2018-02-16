CC = g++
FLAGS = -Wall

all:	TestSockets

TestSockets:	Socket.o Socket.h ServerSocket.o ServerSocket.h
	$(CC) $(FLAGS) ServerSocket.o Socket.o test.cpp -o TestSockets

Socket:	Socket.o Socket.h

ServerSocket:	ServerSocket.o ServerSocket.h

Socket.o:	Socket.cpp Socket.h
	$(CC) $(FLAGS) -c Socket.cpp

ServerSocket.o:	ServerSocket.cpp ServerSocket.h
	$(CC) $(FLAGS) -c ServerSocket.cpp

clean:
	rm *.o
