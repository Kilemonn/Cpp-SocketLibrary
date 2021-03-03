
#include <cassert>
#include <stdexcept>

#include "../socket/Socket.h"

#include "TestUtil.hpp"

const int PORT_NUMBER = 12345;
const std::string LOCALHOST = "127.0.0.1";

/**
 * Test the kt::Socket constructors and exception handling for UDP. This covers the following scenarios:
 * - Constructing a socket and ensuring its default values are set correctly
 */
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

/**
 * Test the kt::Socket constructors and exception handling. This covers the following scenarios:
 * - Test bindUdpSocket()
 * - Test ready()
 * - Test isBound()
 * - Test send()
 * - Test receiveAmount()
 * - Test receiveAll()
 * - Test getLastRecievedAddress()
 * - Test receiveToDelimiter
 * - Test get()
 */
void testWifiFunctions()
{
    preFunctionTest(__func__);

    kt::Socket server(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::UDP);

    assert(!server.isBound());
    server.bindUdpSocket();
    assert(server.isBound());

    kt::Socket client(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::UDP);

    const std::string testString = "test";
    assert(client.send(testString));

    // Test receiveAmount()
    assert(server.ready());
    std::string response = server.receiveAmount(testString.size());
    assert(!server.ready());
    assert(testString == response);

    // Test getLastRecievedAddress()
    assert(server.getLastRecievedAddress() == LOCALHOST);

    // Test receiveAll
    const std::string anotherTest = "AnotherOnE";
    assert(server.send(anotherTest));
    assert(client.ready());
    response = client.receiveAmount(anotherTest.size());
    assert(!client.ready());
    assert(anotherTest == response);

    // Test receiveToDelimiter()
    const char delimiter = '~';
    client.send(testString + delimiter);
    assert(server.ready());
    response = server.receiveToDelimiter(delimiter);
    assert(!server.ready());
    assert(response == testString);

    // Test get method
    const std::string x = "x";
    assert(server.send(x));
    assert(client.ready());
    response = client.get();
    assert(response == x);

    const std::string a = "a";
    assert(!server.ready());
    assert(client.send(a));
    assert(server.ready());
    response = server.get();
    assert(response == a);   

    server.close();
    assert(!server.isBound());
    client.close();
}

int main()
{
    testFunction(testWifiConstructors);
    testFunction(testWifiFunctions);
}
