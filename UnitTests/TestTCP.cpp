
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

/**
 * Test the kt::Socket constructors and exception handling. This covers the following scenarios:
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

    // Test that creating a socket will fail to be created if the server is not listening
    assert(throwsException<kt::SocketException>([] 
    {
        kt::Socket socket(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::TCP);
    }));

    // Test copy constructor by creating a copy of the initalised socket and ensuring the server receives the
    // data successfully tests assignment operator
    kt::ServerSocket server(kt::SocketType::Wifi, PORT_NUMBER);
    kt::Socket socket(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::TCP);

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
}

void testWifiServerSocketConstructors()
{
    preFunctionTest(__func__);

    kt::ServerSocket server(kt::SocketType::Wifi, PORT_NUMBER);

    // Ensure a binding exception is thrown if another process, (in this case another server) is using the port
    assert(throwsException<kt::BindingException>([] 
    {
        kt::ServerSocket server2(kt::SocketType::Wifi, PORT_NUMBER);
    }));

    // Ensure Close method works as expected
    server.close();
    // By closing the initial server the second server should be constructed successfully
    kt::ServerSocket server2(kt::SocketType::Wifi, PORT_NUMBER);

    // Check copy constructor by making sure a client can connect and send a message successfully
    kt::ServerSocket server3(server2);

    kt::Socket client(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::TCP);

    kt::Socket serverSocket = server3.accept();
    const std::string testString = "I'm Too Hot!";

    assert(client.send(testString));
    const std::string responseString = serverSocket.receiveAmount(testString.size());

    assert(responseString == testString);

    server3.close();
    serverSocket.close();
    client.close();

    // Test closing the socket of two copied servers will result in a client being unable to connect
    // meaning, "both" server sockets have been closed, make sure a copied server object cannot accept
    kt::ServerSocket initalServer(kt::SocketType::Wifi, PORT_NUMBER);

    assert(throwsException<kt::SocketException>([&initalServer] 
    {
        kt::ServerSocket actualServer = initalServer;
        initalServer.close();
        actualServer.accept();
    }));

    assert(throwsException<kt::SocketException>([] 
    {
        kt::Socket client = kt::Socket(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::TCP);
    }));
}

void testWifiSocketMethods()
{
    preFunctionTest(__func__);

    kt::ServerSocket server(kt::SocketType::Wifi, PORT_NUMBER);
    kt::Socket client(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::TCP);

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

    assert(!client.ready());
    assert(serverSocket.send(testString + testString + delimiter));
    assert(client.ready());
    response = client.receiveAll();
    assert(!client.ready());
    assert(response == testString + testString + delimiter);

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

int main()
{
    testFunction(testWifiSocketConstructors);
    testFunction(testWifiServerSocketConstructors);
    testFunction(testWifiSocketMethods);
}
