
#include <cassert>
#include <stdexcept>

#include "../src/socket/Socket.h"
#include "../src/serversocket/ServerSocket.h"
#include "../src/socketexceptions/SocketException.hpp"
#include "../src/socketexceptions/BindingException.hpp"

#include "TestUtil.hpp"

const int PORT_NUMBER = 12345;
const std::string LOCALHOST = "127.0.0.1";

/**
 * Test the kt::Socket constructors and exception handling for TCP. This covers the following scenarios:
 * - Wifi Socket created without a specified kt::SocketProtocol
 * - Socket created without a hostname provided
 * - Socket created when no server is listening
 * - The copy constructor and assignment operator allows the copied socket to successfully send data to the receiver
 */
void testWifiSocketConstructors()
{
    preFunctionTest(__func__);

    // Test that a Wifi socket with a protocol set to None will fail
    assert(throwsException<kt::SocketException>([] 
    {
        kt::Socket failedSocket(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi);
    }));

    // Test that a socket created with no hostname fails
    assert(throwsException<kt::SocketException>([] 
    {
        kt::Socket socket("", PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::TCP);
    }));

    // Test that creating a socket will fail to be created if there is no server listening
    assert(throwsException<kt::SocketException>([] 
    {
        kt::Socket socket(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::TCP);
    }));

    // Test copy constructor by creating a copy of the initalised socket and ensuring the server receives the
    // data successfully tests assignment operator
    kt::ServerSocket server(kt::SocketType::Wifi, PORT_NUMBER);
    assert(server.getType() == kt::SocketType::Wifi);
    kt::Socket socket(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::TCP);
    assert(socket.getType() == kt::SocketType::Wifi);
    assert(socket.getProtocol() == kt::SocketProtocol::TCP);

    assert(socket.getAddress() == LOCALHOST);
    assert(socket.getPort() == PORT_NUMBER);

    // Accept connection request from "socket"
    kt::Socket serverSocket = server.accept();
    // Check address and port of serverSocket, in this case it is connected to localhost
    assert(serverSocket.getAddress() == LOCALHOST);
    assert(serverSocket.getPort() != PORT_NUMBER);
    // Create copy of "socket" (assignment operator test)
    kt::Socket copiedSocket(socket);

    // Send test string to server socket
    const std::string testString = "Test";
    assert(copiedSocket.send(testString));
    // Receive test string
    const std::string response = serverSocket.receiveAmount(testString.size());
    // Compare test string
    assert(response == testString);

    // Close all sockets
    serverSocket.close();
    copiedSocket.close();
    socket.close();
    server.close();

    // Should fail as all server sockets are closed
    assert(throwsException<kt::SocketException>([] 
    {
        kt::Socket(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::TCP);
    }));
}

/**
 * Test the kt::Socket methods for TCP. This covers the following scenarios:
 * - Test the connected() method
 * - Test receiveAmount()
 * - Test receiveToDelimiter()
 * - Test receiveAll()
 * - Test get()
 * - Test send()
 * - Test ready()
 */
void testWifiSocketMethods()
{
    preFunctionTest(__func__);

    const unsigned int PORT = 75634;
    kt::ServerSocket server(kt::SocketType::Wifi, PORT);
    kt::Socket client(LOCALHOST, server.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::TCP);

    kt::Socket serverSocket = server.accept();

    // Test connected function
    assert(client.connected());
    assert(serverSocket.connected());

    // Test receiveAmount method
    const std::string testString = "test";
    assert(!serverSocket.ready());
    assert(client.send(testString));
    assert(serverSocket.ready());
    std::string response = serverSocket.receiveAmount(testString.size());
    assert(response == testString);

    // Test receiveToDelimiter method
    const char delimiter = '!';
    assert(!client.ready());
    assert(serverSocket.send(testString + delimiter));
    assert(client.ready());
    response = client.receiveToDelimiter(delimiter);
    assert(response == testString);

    // Test receiveAll method
    assert(!client.ready());
    assert(serverSocket.send(testString + testString + delimiter));
    assert(client.ready());
    response = client.receiveAll();
    assert(!client.ready());
    assert(response == testString + testString + delimiter);

    // Test get method
    assert(!client.ready());
    assert(serverSocket.send(testString));
    assert(client.ready());
    response = client.get();
    assert(response == "t");

    assert(client.ready());
    response = client.get();
    assert(response == "e");

    assert(client.ready());
    response = client.get();
    assert(response == "s");

    assert(client.ready());
    response = client.get();
    assert(response == "t");
    assert(!client.ready());

    // Check delimiter illegal character
    assert(throwsException<kt::SocketException>([&client] 
    {
        client.receiveToDelimiter('\0');
    }));

    client.close();
    assert(!client.connected());
    // Remote socket cannot tell that the connection has been closed
    // assert(!serverSocket.connected()); // -> fails here
    serverSocket.close();
    server.close();
}

/**
 * The main used to call the TCP test functions.
 */
int main()
{
    testFunction(testWifiSocketConstructors);
    testFunction(testWifiServerSocketConstructors);
    testFunction(testWifiSocketMethods);
}
