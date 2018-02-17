CC = g++
PREFLAGS = -Wall -std=c++11 -pthread -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -L/usr/lib/x86_64-linux-gnu -lglib-2.0 -L/usr/lib/x86_64-linux-gnu -lgio-2.0
POSTFLAGS = -lbluetooth

all:	TestSockets

TestSockets:	Socket.o Socket.h ServerSocket.o ServerSocket.h
	$(CC) $(PREFLAGS) ServerSocket.o Socket.o test.cpp $(POSTFLAGS)

Socket:	Socket.o Socket.h

ServerSocket:	ServerSocket.o ServerSocket.h

Socket.o:	Socket.cpp Socket.h
	$(CC) $(PREFLAGS) -c Socket.cpp $(POSTFLAGS)

ServerSocket.o:	ServerSocket.cpp ServerSocket.h
	$(CC) $(PREFLAGS) -c ServerSocket.cpp $(POSTFLAGS)

clean:
	rm *.o

rebuild: clean TestSockets
