
#include <iostream>
#include <thread>

#include "Socket/Socket.h"
#include "ServerSocket/ServerSocket.h"

void wifiFunction();
void bluetoothFunction();

void testWifi();
void testBluetooth();

int main()
{
	Socket::scanDevices();

	testWifi();

	testBluetooth();

	return 0;
}

void testWifi()
{
	std::cout << "TESTING WIFI" << std::endl;
	std::thread t1(wifiFunction);

	Socket socket("127.0.0.1", 3000, true);
	std::cout << "Connected" << std::endl;

	std::string received = socket.receiveToDelimiter('\n');
	std::cout << "RECIEVED: " << received << std::endl;

	socket.sendMessage("12345");
	socket.closeSocket();

	t1.join();
}

void wifiFunction()
{
	ServerSocket server(3000, true);

	Socket client(server.acceptWifiConnection());
	std::cout << "Accepted" << std::endl;

	client.sendMessage("HEY\n");

	std::string res = client.receiveAmount(4);
	std::cout << "RES: " << res << std::endl;
}

void testBluetooth()
{
	std::cout << "TESTING BLUETOOTH" << std::endl;
	std::thread t1(bluetoothFunction);

	Socket socket("08:6A:0A:DF:8F:B6", 1, false);
	std::cout << "Connected" << std::endl;

	std::string received = socket.receiveToDelimiter('\n');
	std::cout << "RECIEVED: " << received << std::endl;

	socket.sendMessage("12345");
	socket.closeSocket();

	t1.join();
}

void bluetoothFunction()
{
	ServerSocket server(1, false);

	Socket client(server.acceptWifiConnection());
	std::cout << "Accepted" << std::endl;

	client.sendMessage("HEY\n");

	std::string res = client.receiveAmount(4);
	std::cout << "RES: " << res << std::endl;
}
