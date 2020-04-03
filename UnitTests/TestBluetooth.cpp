
#include <iostream>
#include <fstream>
#include <thread>
#include <exception>
#include <cassert>
#include <stdexcept>
#include <chrono>

#include "../Socket/Socket.h"
#include "../ServerSocket/ServerSocket.h"
#include "../SocketExceptions/SocketException.hpp"
#include "../SocketExceptions/BindingException.hpp"
#include "../SocketExceptions/TimeoutException.hpp"
#include "../Enums/SocketProtocol.cpp"
#include "../Enums/SocketType.cpp"

void testBluetooth();
void doScan();

const std::string bluetoothLocalAddress = "B8:27:EB:99:F4:E6";

int main()
{
	try
	{
		std::system("sudo hciconfig hci0 piscan");

		// doScan();

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

void testBluetooth()
{
	std::cout << "\nTESTING BLUETOOTH\n";

	kt::ServerSocket server(kt::SocketType::Bluetooth);
	kt::Socket socket(bluetoothLocalAddress, server.getPort(), kt::SocketType::Bluetooth);
	std::cout << "(BT) Connected\n";

	kt::Socket client = server.accept();
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

	socket.close();
	server.close();
}
