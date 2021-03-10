
#include <cassert>
#include <stdexcept>

#include "../socket/Socket.h"
#include "../serversocket/ServerSocket.h"
#include "../socketexceptions/SocketException.hpp"
#include "../socketexceptions/BindingException.hpp"

#include "TestUtil.hpp"
#include "TestClass.h"
#include "TestClassSerialiser.h"

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
 * Test the kt::ServerSocket constructors and exception handling for TCP. This covers the following scenarios:
 * - ServerSocket created while another process is using the specified port number
 * - Test closing operation
 * - Test copy constructor and assignment operator
 * - Test closing a copied server will also close the initial server since they are both using the same port
 */
void testWifiServerSocketConstructors()
{
    preFunctionTest(__func__);

    const unsigned int PORT = 87682;
    kt::ServerSocket server(kt::SocketType::Wifi, PORT);

    // Ensure a binding exception is thrown if another process, (in this case another server) is using the port
    assert(throwsException<kt::BindingException>([&server] 
    {
        kt::ServerSocket server2(kt::SocketType::Wifi, server.getPort());
    }));

    // Check copy constructor by making sure a client can connect and send a message successfully
    kt::ServerSocket server2(server);

    kt::Socket client(LOCALHOST, server.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::TCP);

    kt::Socket serverSocket = server2.accept();
    const std::string testString = "I'm Too Hot!";

    assert(client.send(testString));
    const std::string responseString = serverSocket.receiveAmount(testString.size());
    assert(responseString == testString);

    serverSocket.close();
    client.close();
    server2.close();
    server.close();

    // Test closing the socket of two copied servers will result in a client being unable to connect
    // meaning, "both" server sockets have been closed, make sure a copied server object cannot accept
    const unsigned int PORT_3 = 97654;
    kt::ServerSocket initalServer(kt::SocketType::Wifi, PORT_3);
    assert(throwsException<kt::SocketException>([&initalServer] 
    {
        kt::ServerSocket actualServer = initalServer;
        initalServer.close();
        actualServer.accept();
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

void testSendAndReceiveObject()
{
    preFunctionTest(__func__);
    kt::ServerSocket server(kt::SocketType::Wifi);

    kt::Socket sender(LOCALHOST, server.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::TCP);
    kt::Socket receiver = server.accept();

    assert(sender.connected());
    assert(receiver.connected());

    int ints[] = {1, 2, 3, 42, 5, 61, 7, 8, 100};
    char* chars = (char *) &"this is a test";
    std::string str = "TEST";
    kt::TestClassSerialiser serialiser;
    kt::TestClass test(ints, chars, str);

    assert(sender.sendObject(test, dynamic_cast<kt::TestClassSerialiser::SocketSerialiser*>(&serialiser)));
    assert(receiver.ready());
    kt::TestClass result = receiver.receiveObject(dynamic_cast<kt::TestClassSerialiser::SocketSerialiser*>(&serialiser));

    std::cout << "Res str: " << result.str << " --- test str: " << test.str << std::endl;
    assert(result.str == test.str);

    receiver.close();
    sender.close();
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
    testFunction(testSendAndReceiveObject);
}
