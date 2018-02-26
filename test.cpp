
#include <iostream>
#include <thread>
#include <stdexcept>

#include "Socket/Socket.h"
#include "ServerSocket/ServerSocket.h"
#include "SocketExceptions/SocketError.hpp"
#include "SocketExceptions/BindingError.hpp"

void wifiFunction(int const &);
void bluetoothFunction(int const &);

void testWifi();
void testBluetooth();

void doScan();

int main()
{
	try
	{
		std::system("sudo hciconfig hci0 piscan");

		doScan();

		testWifi();

		testBluetooth();
	}
	catch (const SocketError& se)
	{
		std::cout << se.what() << std::endl;
	}

	return 0;
}

void doScan()
{
	std::vector<std::pair<std::string, std::string> > devices = Socket::scanDevices();

	for (unsigned int i = 0; i < devices.size(); i++)
	{
		std::cout << i << " - " << devices[i].first << " -> " << devices[i].second << "\n";
	}
}

void testWifi()
{
	std::cout << "\nTESTING WIFI\n";

	ServerSocket server(ServerSocket::WIFI);

	int p = server.getPort();
	std::thread t1(wifiFunction, p);

	Socket client(server.accept());
	std::cout << "Accepted\n";

	if (client.send("HEY\n"))
	{
		std::cout << "SENT! (Wifi)\n";
	}
	else
	{
		std::cout << "NOT SENT! (Wifi)\n";
	}

	std::string res = client.receiveAmount(2);
	std::cout << "RES: " << res << std::endl;

	res = client.receiveAmount(2);
	std::cout << "RES: " << res << std::endl;

	client.send("DAMN SON!");

	client.send("HI");

	t1.join();
}

void wifiFunction(int const & p)
{
	Socket socket("127.0.0.1", p, Socket::WIFI);
	std::cout << "Connected\n";

	std::string received = socket.receiveToDelimiter('\n');
	std::cout << "RECIEVED: " << received << std::endl;

	socket.send("12345");

	received = socket.receiveToDelimiter(' ');
	std::cout << "RECIEVED: " << received << std::endl;

	received = socket.receiveToDelimiter('!');
	std::cout << "RECIEVED: " << received << std::endl;
	
	while (socket.ready())
	{
		char c = socket.get();
		std::cout << "Char c: " << c << "\n";	
	}
}

void testBluetooth()
{
	std::cout << "\nTESTING BLUETOOTH\n";

	ServerSocket server(ServerSocket::BLUETOOTH, 3);

	int p = server.getPort();
	std::thread t1(bluetoothFunction, p);

	Socket client(server.accept());
	std::cout << "(BT) Accepted\n";

	if (client.send("HEY\n"))
	{
		std::cout << "SENT! (BT)\n";
	}
	else
	{
		std::cout << "NOT SENT! (BT)\n";
	}

	std::string res = client.receiveAmount(4);
	std::cout << "(BT) RES: " << res << std::endl;

	t1.join();
}

void bluetoothFunction(int const & p)
{
	Socket socket("B8:27:EB:99:F4:E6", p, Socket::BLUETOOTH);
	std::cout << "(BT) Connected\n";

	std::string received = socket.receiveToDelimiter('\n');
	std::cout << "(BT) RECIEVED: " << received << std::endl;

	socket.send("12345");
}
