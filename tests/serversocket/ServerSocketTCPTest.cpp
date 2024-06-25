
#include <chrono>

#include <gtest/gtest.h>

#include "../../src/serversocket/ServerSocket.h"
#include "../../src/socketexceptions/BindingException.hpp"
#include "../../src/socketexceptions/TimeoutException.hpp"

const int PORT_NUMBER = 87682;

namespace kt
{
    class ServerSocketTCPTest: public ::testing::Test
    {
    protected:
        ServerSocket serverSocket;
    protected:
        ServerSocketTCPTest() : serverSocket(SocketType::Wifi, PORT_NUMBER) {}
        // void SetUp() override { }
        void TearDown() override
        {
            serverSocket.close();
        }
    };

    /*
     * Ensure the defaults are set correctly.
     */
    TEST_F(ServerSocketTCPTest, TestDefaultConstructor)
    {
        ASSERT_EQ(SocketType::Wifi, serverSocket.getType());
        ASSERT_NE(InternetProtocolVersion::Any, serverSocket.getInternetProtocolVersion());
        ASSERT_EQ(PORT_NUMBER, serverSocket.getPort());
    }

    /*
     * Ensure that we throw a binding exception if the port is already used.
     */
    TEST_F(ServerSocketTCPTest, TestConstructors)
    {
        EXPECT_THROW({
            ServerSocket server2(SocketType::Wifi, serverSocket.getPort());
        }, BindingException);
    }

    /*
     * Ensure thatt if a copied serversocket is closed, that it closes the copied socket too since they shared the same underlying descriptor.
     */
    TEST_F(ServerSocketTCPTest, TestCloseCopiedServerSocket)
    {
        ServerSocket copiedServer = serverSocket;
        serverSocket.close();
        ASSERT_THROW(copiedServer.acceptTCPConnection(), SocketException);
    }

    /*
     * Ensure the copy constructed server socket is able to connect to the client and receive messages.
     */
    TEST_F(ServerSocketTCPTest, TestCopyConstructor)
    {
        ServerSocket server2(serverSocket);

        TCPSocket client("127.0.0.1", serverSocket.getPort());

        TCPSocket serverClient = server2.acceptTCPConnection();
        const std::string testString = "test";

        ASSERT_TRUE(client.send(testString));
        const std::string responseString = serverClient.receiveAmount(testString.size());
        ASSERT_EQ(responseString, testString);

        serverClient.close();
        client.close();
        server2.close();
    }

    /*
     * Ensure the server socket can be created using IPV6 and can accept a connection.
     */
    TEST_F(ServerSocketTCPTest, TestServerSocketAsIPV6)
    {
        ServerSocket ipv6Server(SocketType::Wifi, 0, 20, InternetProtocolVersion::IPV6);

        TCPSocket client("0000:0000:0000:0000:0000:0000:0000:0001", ipv6Server.getPort());
        TCPSocket serverClient = ipv6Server.acceptTCPConnection();

        const std::string testString = "test";
        ASSERT_TRUE(client.send(testString));
        const std::string responseString = serverClient.receiveAmount(testString.size());
        ASSERT_EQ(responseString, testString);

        serverClient.close();
        client.close();
        ipv6Server.close();
    }

    /*
     * Ensure the server socket cannot be connected to by a client using IPV4.
     */
    TEST_F(ServerSocketTCPTest, TestServerSocketAsIPV4ServerAndIPV4Client)
    {
        serverSocket.close();

        ServerSocket ipv4Server(SocketType::Wifi, 0, 20, InternetProtocolVersion::IPV4);
        ASSERT_EQ(InternetProtocolVersion::IPV4, ipv4Server.getInternetProtocolVersion());

        // Attempt to connect to a local server using a IPV6 address (which is not being hosted)
        EXPECT_THROW({
            TCPSocket client("::1", ipv4Server.getPort());
        }, SocketException);
        
        // Make sure theres no incoming connections
        EXPECT_THROW({
            ipv4Server.acceptTCPConnection(1000);
        }, TimeoutException);

        ipv4Server.close();
    }

    /**
     * Ensure that calls to ServerSocket.accept() with a provided timeout waits for atleast the provided timeout before throwing a TimeoutException.
    */
    TEST_F(ServerSocketTCPTest, TestServerSocketAcceptTimeout)
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        EXPECT_THROW({
            TCPSocket serverClient = serverSocket.acceptTCPConnection(1 * 1000000);
        }, TimeoutException);

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        ASSERT_GE(1, std::chrono::duration_cast<std::chrono::seconds>(end - start).count());
    }
}
