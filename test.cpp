
#include <iostream>
#include <thread>
#include <exception>
#include <stdexcept>
#include <assert.h>

#include "Socket/Socket.h"
#include "ServerSocket/ServerSocket.h"
#include "SocketExceptions/SocketError.hpp"
#include "SocketExceptions/BindingError.hpp"

void wifiClient(unsigned int const &);
void bluetoothFunction(unsigned int const &);

void testWifi();
void testBluetooth();

void doScan();

const std::string bluetoothLocalAddress = "B8:27:EB:99:F4:E6";

int main()
{

	// std::system("sudo hciconfig hci0 piscan");

	// doScan();

	testWifi();

	// testBluetooth();

	#ifdef _WIN32

    // WSACleanup();

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

void testWifi()
{
	std::cout << "\nTESTING WIFI\n";
	bool error = false;

	try
	{
		kt::ServerSocket server(kt::ServerSocket::WIFI);

		unsigned int p = server.getPort();
		std::thread t1(wifiClient, p);

		kt::Socket client(server.accept());
		std::cout << "Accepted\n";

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

		client.send("DAMN SON!");

		client.send("Testing Wifi Stuff");

		t1.join();
	}
	catch (const SocketError& se)
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

void wifiClient(unsigned int const & p)
{
	kt::Socket socket("127.0.0.1", p, kt::Socket::WIFI);
	std::cout << "Connected\n";

	char delimiter = '\n';
	std::string received = socket.receiveToDelimiter(delimiter);
	assert(received[received.size() - 1] != delimiter);
	std::cout << "RECIEVED: " << received << std::endl;

	socket.send("12345");

	delimiter = ' ';
	received = socket.receiveToDelimiter(delimiter);
	assert(received[received.size() - 1] != delimiter);
	std::cout << "RECIEVED: " << received << std::endl;

	delimiter = '!';
	received = socket.receiveToDelimiter(delimiter);
	assert(received[received.size() - 1] != delimiter);
	std::cout << "RECIEVED: " << received << std::endl;
	
	received = socket.receiveAll();
	std::cout << "RECIEVED: " << received << std::endl;
}

void testBluetooth()
{
	std::cout << "\nTESTING BLUETOOTH\n";

	kt::ServerSocket server(kt::ServerSocket::BLUETOOTH, 3);

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

void bluetoothFunction(unsigned int const & p)
{
	kt::Socket socket(bluetoothLocalAddress, p, kt::Socket::BLUETOOTH);
	std::cout << "(BT) Connected\n";

	std::string received = socket.receiveToDelimiter('\n');
	std::cout << "(BT) RECIEVED: " << received << std::endl;

	socket.send("12345");
}
