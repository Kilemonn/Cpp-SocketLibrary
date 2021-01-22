CC = g++
PREFLAGS = -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -L/usr/lib/x86_64-linux-gnu -lglib-2.0 -L/usr/lib/x86_64-linux-gnu -lgio-2.0
POSTFLAGS = -lbluetooth -Wall -std=c++11 -pthread

all:	TestTCP TestUDP TestBluetooth

TestTCP:	Socket.o src/socket/Socket.h ServerSocket.o src/serversocket/ServerSocket.h
	$(CC) $(PREFLAGS) ServerSocket.o Socket.o src/tests/TestTCP.cpp -o TestTCP $(POSTFLAGS)

TestUDP:	Socket.o src/socket/Socket.h ServerSocket.o src/serversocket/ServerSocket.h
	$(CC) $(PREFLAGS) ServerSocket.o Socket.o src/tests/TestUDP.cpp -o TestUDP $(POSTFLAGS)

TestBluetooth:	Socket.o src/socket/Socket.h ServerSocket.o src/serversocket/ServerSocket.h
	$(CC) $(PREFLAGS) ServerSocket.o Socket.o src/tests/TestBluetooth.cpp -o TestBluetooth $(POSTFLAGS)

Socket.o:	src/socket/Socket.cpp src/socket/Socket.h
	$(CC) $(PREFLAGS) -c src/socket/Socket.cpp $(POSTFLAGS)

ServerSocket.o:	src/serversocket/ServerSocket.cpp src/serversocket/ServerSocket.h
	$(CC) $(PREFLAGS) -c src/serversocket/ServerSocket.cpp $(POSTFLAGS)

clean:
	rm *.o

check: CheckTestTCP CheckTestUDP CheckTestBluetooth

CheckTestTCP:	TestTCP
	./TestTCP

CheckTestUDP:	TestUDP
	./TestUDP

CheckTestBluetooth:	TestBluetooth
	./TestBluetooth

memcheck: MemCheckTestTCP MemCheckTestUDP MemCheckTestBluetooth

MemCheckTestTCP:	TestTCP
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -v ./TestTCP

MemCheckTestUDP:	TestUDP
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -v ./TestUDP

MemCheckTestBluetooth:	TestBluetooth
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all -v ./TestBluetooth

rebuild: clean all
