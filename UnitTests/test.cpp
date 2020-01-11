
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

void TCPClient(const unsigned int&);
void UDPClient(const unsigned int&);
void doTest();

void wifiClient(const unsigned int &);
void bluetoothFunction(const unsigned int&);

void testTCP();
void testUDP();
void testBluetooth();

void doScan();

const std::string bluetoothLocalAddress = "B8:27:EB:99:F4:E6";

int main()
{
	try
	{
		// std::system("sudo hciconfig hci0 piscan");

		// doScan();

		doTest();

		testTCP();

		testUDP();

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

void doTest()
{
	kt::ServerSocket server(kt::SocketType::Wifi, 12345);

	try
	{
		kt::Socket socket = server.accept(100000);
		assert(false);
	}
	catch (const kt::TimeoutException &e)
	{
		assert(true);
	}

	kt::Socket socket("127.0.0.1", 12345, kt::SocketType::Wifi, kt::SocketProtocol::TCP);

	try
	{
		kt::Socket socket = server.accept();
		assert(true);
	}
	catch (const std::exception &e)
	{
		assert(false);
	}

	socket.close();
	server.close();
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

	try
	{
		kt::ServerSocket server(kt::SocketType::Wifi);

		unsigned int p = server.getPort();
		std::thread t1(TCPClient, p);

		kt::Socket client = server.accept();
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

		client.send("Testing Wifi receiveAll\n");

		std::cout << "WAITING..." << std::endl;

		t1.join();
	}
	catch (const kt::SocketException& se)
	{
		std::cout << se.what() << std::endl;
		assert(false);
	}
	catch (...)
	{
		std::cout << "CAUGHT IT" << std::endl;
		assert(false);
	}
}

void TCPClient(const unsigned int& p)
{
	kt::Socket socket("127.0.0.1", p, kt::SocketType::Wifi, kt::SocketProtocol::TCP);
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
	
	received = socket.receiveAll();
	assert(received == "Testing Wifi receiveAll\n");
	std::cout << "RECIEVED4: " << received << std::endl;

	// while(socket.connected())
	// {
	// 	// Don't leave until the other socket has been closed
	// }
}

void testUDP()
{
	std::cout << "Testing UDP: " << std::endl;

	try
	{
		unsigned int port = 65222;
		kt::Socket socket("127.0.0.1", port, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
		std::cout << "First UDP Client created.\n";

		socket.bind();

		std::cout << "UDP Read: " + socket.get() << std::endl;
		socket.send("test\n");

		if (socket.ready(1000000))
		{
			std::cout << "Socket Ready: " + socket.get() << std::endl;
		}
		else
		{
			std::cout << "Socket not ready..." << std::endl;
		}

		socket.unbind();
		
		std::thread t1(UDPClient, port);

		std::this_thread::sleep_for(std::chrono::seconds(3));

		t1.join();
	}
	catch (const kt::SocketException& se)
	{
		std::cout << se.what() << std::endl;
		assert(false);
	}
	catch (...)
	{
		std::cout << "CAUGHT IT" << std::endl;
		assert(false);
	}
}

void UDPClient(const unsigned int& p)
{
	kt::Socket socket("127.0.0.1", p, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
	std::cout << "Connected\n";

	socket.sendTo("test\n");

	socket.close();
}

void testBluetooth()
{
	std::cout << "\nTESTING BLUETOOTH\n";

	kt::ServerSocket server(kt::SocketType::Bluetooth, 5);

	unsigned int p = server.getPort();
	std::thread t1(bluetoothFunction, p);

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


