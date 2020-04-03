
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

int main()
{
	testFunction(testGetLocalMacAddress);
	testFunction(testBluetooth);
}
