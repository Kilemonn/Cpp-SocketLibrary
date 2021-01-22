
#include <iostream>
#include <fstream>
#include <thread>
#include <exception>
#include <cassert>
#include <stdexcept>
#include <chrono>

#include "../socket/Socket.h"
#include "../serversocket/ServerSocket.h"
#include "../socketexceptions/SocketException.hpp"
#include "../socketexceptions/BindingException.hpp"
#include "../socketexceptions/TimeoutException.hpp"
#include "../enums/SocketProtocol.cpp"
#include "../enums/SocketType.cpp"

#include "TestUtil.hpp"

void testBluetooth()
{
	preFunctionTest(__func__);

	kt::ServerSocket server(kt::SocketType::Bluetooth);
	kt::Socket socket(kt::Socket::getLocalMACAddress(), server.getPort(), kt::SocketType::Bluetooth);

	kt::Socket client = server.accept();
	const std::string toSend = "TestBluetooth";

	assert(client.send(toSend));

	std::string res = client.receiveAmount(toSend.size());
	assert(res == toSend);

	socket.close();
	server.close();
}

void testGetLocalMacAddress()
{
	preFunctionTest(__func__);
	
	std::string address = kt::Socket::getLocalMACAddress();
	assert(address != "");
}

void testScan()
{
	preFunctionTest(__func__);

	std::vector<std::pair<std::string, std::string> > devices = kt::Socket::scanDevices();
	for (std::pair<std::string, std::string> p : devices)
	{
		std::cout << p.first << " - " << p.second << std::endl;
	}
}

int main()
{
	testFunction(testGetLocalMacAddress);
	testFunction(testScan);
	testFunction(testBluetooth);
}
