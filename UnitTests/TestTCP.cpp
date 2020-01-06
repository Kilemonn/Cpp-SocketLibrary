
#include <iostream>
#include <fstream>
#include <thread>
#include <exception>
#include <cassert>
#include <stdexcept>
#include <chrono>
#include <typeinfo>

#include "../Socket/Socket.h"
#include "../ServerSocket/ServerSocket.h"
#include "../SocketExceptions/SocketException.hpp"
#include "../SocketExceptions/BindingException.hpp"
#include "../Enums/SocketProtocol.cpp"
#include "../Enums/SocketType.cpp"

const int PORT_NUMBER = 12345;

/**
 * A helper function used to ensure that an exception is thrown by the passed in function and
 * also ensures the exception's type is what is specified in the template argument.
 * 
 * @param function the function to test
 * 
 * @return true if an exception was thrown and was of type T, otherwise false.
 */
template <typename T>
bool throwsException(const std::function<void()> function)
{
    try
    {
        function();
    }
    catch(T ex)
    {
        return true;
    }
    catch (...)
    {
        // Do nothing, will return false
    }
    return false;
}

/**
 * Print the function name to indicate which test function is currently running.
 * 
 * @param functionName the function name to be printed
 */
void preFunctionTest(std::string functionName)
{
    std::cout << "Running... " << functionName << "()... " << std::flush;
}

/**
 * Will run a specific function and print "PASS" once it is finished.
 * 
 * @param function the function to call
 */
void testFunction(std::function<void()> function)
{
    function();
    std::cout << "PASS" << std::endl;
}

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
        kt::Socket failedSocket("127.0.0.1", kt::SocketType::Wifi, PORT_NUMBER);
    }));

    // Test that a socket created with no hostname fails
    assert(throwsException<kt::SocketException>([] 
    {
        kt::Socket socket("", kt::SocketType::Wifi, PORT_NUMBER, kt::SocketProtocol::TCP);
    }));

    // Test that creating a socket will fail to be created if the server is not listening
    assert(throwsException<kt::SocketException>([] 
    {
        kt::Socket socket("127.0.0.1", kt::SocketType::Wifi, PORT_NUMBER, kt::SocketProtocol::TCP);
    }));

    // Test copy constructor by creating a copy of the initalised socket and ensuring the server receives the
    // data successfully tests assignment operator
    kt::ServerSocket server(kt::SocketType::Wifi, PORT_NUMBER);
    kt::Socket socket("127.0.0.1", kt::SocketType::Wifi, PORT_NUMBER, kt::SocketProtocol::TCP);

    // Accept connection request from "socket"
    kt::Socket serverSocket = server.accept();
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
}

int main()
{
    testFunction(testWifiSocketConstructors);
    testFunction(testWifiServerSocketConstructors);
}
