CC = g++
PREFLAGS = -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -L/usr/lib/x86_64-linux-gnu -lglib-2.0 -L/usr/lib/x86_64-linux-gnu -lgio-2.0
POSTFLAGS = -lbluetooth -Wall -std=c++11 -pthread

all:	TestSockets UnitTests

TestSockets:	Socket.o Socket/Socket.h ServerSocket.o ServerSocket/ServerSocket.h
	$(CC) $(PREFLAGS) ServerSocket.o Socket.o test.cpp -o TestSockets $(POSTFLAGS)

UnitTests:	Socket.o Socket/Socket.h ServerSocket.o ServerSocket/ServerSocket.h
	$(CC) $(PREFLAGS) ServerSocket.o Socket.o UnitTests/TestTCP.cpp -o TestTCP $(POSTFLAGS)

Socket.o:	Socket/Socket.cpp Socket/Socket.h
	$(CC) $(PREFLAGS) -c Socket/Socket.cpp $(POSTFLAGS)

ServerSocket.o:	ServerSocket/ServerSocket.cpp ServerSocket/ServerSocket.h
	$(CC) $(PREFLAGS) -c ServerSocket/ServerSocket.cpp $(POSTFLAGS)

clean:
	rm *.o

rebuild: clean TestSockets UnitTests
