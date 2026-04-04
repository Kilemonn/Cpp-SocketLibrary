
#include <gtest/gtest.h>

#include "../../src/socket/UDPSocket.h"
#include "../../src/serversocket/TCPServerSocket.h"
#include "../../src/ipc/IPCServerSocket.h"

namespace kt
{
    /**
     * Ensure that we can bind to the same port using TCP and UDP sockets.
     * 
     * In this case, when UDP binds first.
     */
    TEST(ScenarioTest, UDPThenTCPBindSamePort)
    {
        kt::UDPSocket socket;
        std::pair<int, kt::SocketAddress> bindResult = socket.bind();
        ASSERT_EQ(0, bindResult.first);
        ASSERT_NE(std::nullopt, socket.getListeningPort());

        kt::TCPServerSocket server(std::nullopt, socket.getListeningPort().value());

        ASSERT_EQ(server.getPort(), socket.getListeningPort().value());

        server.close();
        socket.close();
    }

    /**
     * Ensure that we can bind to the same port using TCP and UDP sockets.
     * 
     * In this case, when TCP binds first.
     */
    TEST(ScenarioTest, TCPThenUDPBindSamePort)
    {
        kt::TCPServerSocket server;

        kt::UDPSocket socket;
        std::pair<int, kt::SocketAddress> bindResult = socket.bind(std::nullopt, server.getPort());
        ASSERT_EQ(0, bindResult.first);
        ASSERT_NE(std::nullopt, socket.getListeningPort());

        ASSERT_EQ(server.getPort(), socket.getListeningPort().value());

        server.close();
        socket.close();
    }

    /**
     * Ensure that we can bind two UDP sockets to the same address by setting SO_REUSEADDR in the pre-bind handler.
     */
    TEST(ScenarioTest, TwoUDPSocketsBindingToSamePort)
    {
        std::function setReuseAddrOption = [](SOCKET& s) {
            const int enableOption = 1;
            ASSERT_EQ(0, setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&enableOption, sizeof(enableOption)));
        };

        kt::UDPSocket socket;
        std::pair<int, kt::SocketAddress> bindResult = socket.bind(std::nullopt, 0, setReuseAddrOption);
        ASSERT_EQ(0, bindResult.first);

        kt::UDPSocket socket2;
        bindResult = socket2.bind(std::nullopt, socket.getListeningPort().value(), setReuseAddrOption);
        ASSERT_EQ(0, bindResult.first);

        ASSERT_EQ(socket.getListeningPort().value(), socket2.getListeningPort().value());

        kt::UDPSocket sendSocket;
        const std::string data = "TwoUDPSocketsBindingToSamePort";
        std::pair<int, kt::SocketAddress> sendResult = sendSocket.sendTo("localhost", socket.getListeningPort().value(), data);
        ASSERT_NE(0, sendResult.first);

        // Make sure only one of the sockets is ready to read, not both
        ASSERT_FALSE(socket.ready() && socket2.ready());
        ASSERT_TRUE(socket2.ready() || socket.ready());

        socket.close();
        socket2.close();

        sendSocket.close();
    }

    // The tcp README example
    TEST(ScenarioTest, TcpExampleTest)
    {
        // Create a new TCP ServerSocket
        kt::TCPServerSocket server(std::nullopt, 56756, 20, kt::InternetProtocolVersion::IPV6);

        // Create new TCP socket
        kt::TCPSocket client("::1", server.getPort());

        // Accept the incoming connection at the server
        kt::TCPSocket serverSocket = server.accept();

        // Send string with text before and after the delimiter
        const std::string testString = "TCP Delimiter Test";
        const char delimiter = '~';
        if (client.send(testString + delimiter + "other string") == 0)
        {
            std::cout << "Failed to send test string" << std::endl;
            return;
        }

        if (serverSocket.ready())
        {
            std::string response = serverSocket.receiveToDelimiter(delimiter);
            // Check that the received string is the same as the string sent by the client
            ASSERT_EQ(response, testString);
        }

        // Close all sockets
        client.close();
        server.close();
        serverSocket.close();
    }

    // The udp README example
    TEST(ScenarioTest, UdpExampleTest)
    {
        // The socket receiving data must first be bound
        kt::UDPSocket socket;
        socket.bind(kt::InternetProtocolVersion::IPV4, std::nullopt, 37893);

        kt::UDPSocket client;
        const std::string testString = "UDP test string";
        if (client.sendTo("localhost", 37893, testString).first == 0)
        {
            std::cout << "Failed to send to address." << std::endl;
            return;
        }

        if (socket.ready())
        {
            std::pair<std::optional<std::string>, std::pair<int, kt::SocketAddress>> recieved = socket.receiveFrom(testString.size());
            ASSERT_EQ(testString, recieved.first.value());
        }

        socket.close();
    }

    // The ipc README example
    TEST(ScenarioTest, IpcExampleTest)
    {
        const std::string ipcChannel = "/tmp/ipcExample.sock";
        // Create a new IPCSocket
        kt::IPCServerSocket server(ipcChannel);

        // Create new IPC socket
        kt::IPCSocket client(ipcChannel);

        // Accept the incoming connection at the server
        kt::IPCSocket serverSocket = server.accept();

        // Send string with text before and after the delimiter
        const std::string testString = "IPC Delimiter Test";
        if (client.send(testString) == 0)
        {
            std::cout << "Failed to send test string" << std::endl;
            return;
        }

        if (serverSocket.ready())
        {
            std::string response = serverSocket.receiveAll();
            // Check that the received string is the same as the string sent by the client
            ASSERT_EQ(response, testString);
        }

        // Close all sockets
        client.close();
        server.close();
        serverSocket.close();
    }
}
