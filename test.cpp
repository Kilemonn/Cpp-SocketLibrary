
#include <iostream>
#include <fstream>
#include <thread>
#include <exception>
#include <stdexcept>
#include <assert.h>

#include "Socket/Socket.h"
#include "ServerSocket/ServerSocket.h"
#include "SocketExceptions/SocketException.hpp"
#include "SocketExceptions/BindingException.hpp"
#include "Enums/SocketProtocol.cpp"
#include "Enums/SocketType.cpp"

void wifiClient(const unsigned int&);
void bluetoothFunction(const unsigned int&);

void testTCP();
void testBluetooth();

void doScan();

const std::string bluetoothLocalAddress = "B8:27:EB:99:F4:E6";

int main()
{
	try
	{
		// std::system("sudo hciconfig hci0 piscan");

		// doScan();

		testTCP();

		std::string addr = kt::Socket::getLocalMACAddress();

		if (addr != "")
		{
			std::cout << addr << std::endl;
		}
		else
		{
			std::cout << "No local MAC address found" << std::endl;
		}

		testBluetooth();
	}
	catch(const kt::SocketException& ex)
	{
		std::cout << ex.what() << std::endl;
	}

	#ifdef _WIN32

    WSACleanup();

    #endif

	return 0;
}

void doScan()
{
	std::vector<std::pair<std::string, std::string> > devices = kt::Socket::scanDevices(1);

	for (unsigned int i = 0; i < devices.size(); i++)
	{
		std::cout << i << " - " << devices[i].first << " -> " << devices[i].second << "\n";
	}
}

void testTCP()
{
	std::cout << "\nTESTING WIFI\n";
	bool error = false;

	try
	{
		kt::ServerSocket server(kt::SocketType::Wifi);

		unsigned int p = server.getPort();
		std::thread t1(wifiClient, p);

		kt::Socket client(server.accept());
		std::cout << "Accepted - " + client.getAddress() + ":" + std::to_string(client.getPort()) + "\n";

		if (client.send("HEY\n"))
		{
			std::cout << "SENT! (Wifi)\n";
		}
		else
		{
			std::cout << "NOT SENT! (Wifi)\n";
		}

		unsigned int amount = 2;
		std::string res = client.receiveAmount(amount);
		assert(res.size() == amount);
		assert(res == "12");
		std::cout << "RES: " << res << std::endl;

		res = client.receiveAmount(amount);
		assert(res.size() == amount);
		std::cout << "RES: " << res << std::endl;

		char c = client.get();
		std::cout << "GET: " << c << std::endl;

		client.send("DAMN SON!");

		client.send("Testing Wifi Stuff\n");

		std::cout << "WAITING..." << std::endl;

		client.close();

		t1.join();
	}
	catch (const kt::SocketException& se)
	{
		std::cout << se.what() << std::endl;
		error = true;
	}
	catch (...)
	{
		error = true;
		std::cout << "CAUGHT IT" << std::endl;
	}

	assert(error == false);
}

void wifiClient(const unsigned int& p)
{
	kt::Socket socket("127.0.0.1", p, kt::SocketType::Wifi);
	std::cout << "Connected\n";

	char delimiter = '\n';
	std::string received = socket.receiveToDelimiter(delimiter);
	assert(received[received.size() - 1] != delimiter);
	std::cout << "RECIEVED1: " << received << std::endl;

	socket.send("12345");

	delimiter = ' ';
	received = socket.receiveToDelimiter(delimiter);
	assert(received[received.size() - 1] != delimiter);
	std::cout << "RECIEVED2: " << received << std::endl;

	delimiter = '!';
	received = socket.receiveToDelimiter(delimiter);
	assert(received[received.size() - 1] != delimiter);
	std::cout << "RECIEVED3: " << received << std::endl;
	
	delimiter = '\n';
	received = socket.receiveToDelimiter(delimiter);
	std::cout << "RECIEVED4: " << received << std::endl;

	while(socket.ready())
	{
		// Don't leave until the other socket has been closed
	}
}

void testBluetooth()
{
	std::cout << "\nTESTING BLUETOOTH\n";

	kt::ServerSocket server(kt::SocketType::Bluetooth, 5);

	unsigned int p = server.getPort();
	std::thread t1(bluetoothFunction, p);

	kt::Socket client(server.accept());
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

void bluetoothFunction(const unsigned int& p)
{
	kt::Socket socket(bluetoothLocalAddress, p, kt::SocketType::Bluetooth);
	std::cout << "(BT) Connected\n";

	std::string received = socket.receiveToDelimiter('\n');
	std::cout << "(BT) RECIEVED: " << received << std::endl;

	socket.send("12345");
}
