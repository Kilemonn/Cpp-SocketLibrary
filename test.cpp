
#include <iostream>
#include <thread>
#include <stdexcept>

#include "Socket/Socket.h"
#include "ServerSocket/ServerSocket.h"

void wifiFunction(int const &);
void bluetoothFunction(int const &);

void testWifi();
void testBluetooth();

void doScan();

int main()
{
	try
	{
		doScan();

		testWifi();

		testBluetooth();
	}
	catch (const std::runtime_error& rex)
	{
		std::cout << rex.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "OK?" << std::endl;
	}

	return 0;
}

void doScan()
{
	std::system("sudo hciconfig hci0 piscan");

	std::vector<std::pair<std::string, std::string> > devices = Socket::scanDevices();

	for (unsigned int i = 0; i < devices.size(); i++)
	{
		std::cout << i << " - " << devices[i].first << " -> " << devices[i].second << "\n";
	}
}

void testWifi()
{
	std::cout << "\nTESTING WIFI\n";

	ServerSocket server(Socket::WIFI);

	int p = server.getPort();
	std::thread t1(wifiFunction, p);

	Socket client(server.acceptConnection());
	std::cout << "Accepted\n";

	if (client.sendMessage("HEY\n"))
	{
		std::cout << "SENT! (Wifi)\n";
	}
	else
	{
		std::cout << "NOT SENT! (Wifi)\n";
	}

	std::string res = client.receiveAmount(4);
	std::cout << "RES: " << res << std::endl;

	t1.join();
}

void wifiFunction(int const & p)
{
	Socket socket("127.0.0.1", p, Socket::WIFI);
	std::cout << "Connected\n";

	std::string received = socket.receiveToDelimiter('\n');
	std::cout << "RECIEVED: " << received << std::endl;

	socket.sendMessage("12345");
	socket.closeSocket();
}

void testBluetooth()
{
	std::cout << "\nTESTING BLUETOOTH\n";

	ServerSocket server(Socket::BLUETOOTH);

	int p = server.getPort();
	std::thread t1(bluetoothFunction, p);

	Socket client(server.acceptConnection());
	std::cout << "(BT) Accepted\n";

	if (client.sendMessage("HEY\n"))
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
	Socket socket("08:6A:0A:DF:8F:B6", p, Socket::BLUETOOTH);
	std::cout << "(BT) Connected\n";

	std::string received = socket.receiveToDelimiter('\n');
	std::cout << "(BT) RECIEVED: " << received << std::endl;

	socket.sendMessage("12345");
	socket.closeSocket();	
}
