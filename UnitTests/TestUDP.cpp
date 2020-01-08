#include <iostream>
#include <fstream>
#include <thread>
#include <exception>
#include <cassert>
#include <stdexcept>
#include <functional>

#include "../Socket/Socket.h"
#include "../ServerSocket/ServerSocket.h"
#include "../SocketExceptions/SocketException.hpp"
#include "../SocketExceptions/BindingException.hpp"
#include "../Enums/SocketProtocol.cpp"
#include "../Enums/SocketType.cpp"

#include "TestUtil.hpp"

const int PORT_NUMBER = 12345;
const std::string LOCALHOST = "127.0.0.1";

void testWifiConstructors()
{
    preFunctionTest(__func__);

    kt::Socket socket(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
    assert(socket.getType() == kt::SocketType::Wifi);
    assert(socket.getProtocol() == kt::SocketProtocol::UDP);
    assert(!socket.connected());
    assert(!socket.ready());
    assert(!socket.isBound());

    socket.close();
}

void testWifiFunctions()
{
    preFunctionTest(__func__);

    kt::Socket server(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
    kt::Socket client(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::UDP);

    assert(!server.isBound());
    server.bind();
    assert(server.isBound());

    const std::string testString = "Test";
    client.sendTo(testString, LOCALHOST);

    assert(server.ready());
    std::string response = server.receiveAmount(testString.size());
    assert(!server.ready());
    assert(testString == response);

    assert(server.getLastRecievedAddress() == LOCALHOST);

    const std::string anotherTest = "AnotherOnE";
    server.sendTo(anotherTest);
    assert(client.ready());
    response = client.receiveAmount(anotherTest.size());
    assert(!client.ready());
    assert(anotherTest == response);

    server.unbind();
    assert(!server.isBound());

    server.close();
    client.close();
}

int main()
{
    testFunction(testWifiConstructors);
    testFunction(testWifiFunctions);
}
