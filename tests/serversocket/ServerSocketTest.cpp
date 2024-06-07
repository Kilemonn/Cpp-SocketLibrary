
#include <chrono>

#include <gtest/gtest.h>

#include "../../src/serversocket/ServerSocket.h"
#include "../../src/socketexceptions/BindingException.hpp"
#include "../../src/socketexceptions/TimeoutException.hpp"

const int PORT_NUMBER = 87682;

namespace kt
{
    class ServerSocketTest: public ::testing::Test
    {
    protected:
        ServerSocket serverSocket;
    protected:
        ServerSocketTest() : serverSocket(SocketType::Wifi, PORT_NUMBER) {}
        // void SetUp() override { }
        void TearDown() override
        {
            serverSocket.close();
        }
    };

    /*
     * Ensure the defaults are set correctly.
     */
    TEST_F(ServerSocketTest, TestDefaultConstructor)
    {
        ASSERT_EQ(SocketType::Wifi, serverSocket.getType());
        ASSERT_NE(InternetProtocolVersion::Any, serverSocket.getInternetProtocolVersion());
        ASSERT_EQ(PORT_NUMBER, serverSocket.getPort());
    }

    /*
     * Ensure that we throw a binding exception if the port is already used.
     */
    TEST_F(ServerSocketTest, TestConstructors)
    {
        EXPECT_THROW({
            ServerSocket server2(SocketType::Wifi, serverSocket.getPort());
        }, BindingException);
    }

    /*
     * Ensure thatt if a copied serversocket is closed, that it closes the copied socket too since they shared the same underlying descriptor.
     */
    TEST_F(ServerSocketTest, TestCloseCopiedServerSocket)
    {
        ServerSocket copiedServer = serverSocket;
        serverSocket.close();
        ASSERT_THROW(copiedServer.accept(), SocketException);
    }

    /*
     * Ensure the copy constructed server socket is able to connect to the client and receive messages.
     */
    TEST_F(ServerSocketTest, TestCopyConstructor)
    {
        ServerSocket server2(serverSocket);

        Socket client("127.0.0.1", serverSocket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::TCP);

        Socket serverClient = server2.accept();
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
    TEST_F(ServerSocketTest, TestServerSocketAsIPV6)
    {
        ServerSocket ipv6Server(SocketType::Wifi, 0, 20, InternetProtocolVersion::IPV6);

        Socket client("0000:0000:0000:0000:0000:0000:0000:0001", ipv6Server.getPort(), SocketType::Wifi, SocketProtocol::TCP);
        Socket serverClient = ipv6Server.accept();

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
    TEST_F(ServerSocketTest, TestServerSocketAsIPV4ServerAndIPV4Client)
    {
        serverSocket.close();

        ServerSocket ipv4Server(SocketType::Wifi, 0, 20, InternetProtocolVersion::IPV4);
        ASSERT_EQ(InternetProtocolVersion::IPV4, ipv4Server.getInternetProtocolVersion());

        // Attempt to connect to a local server using a IPV6 address (which is not being hosted)
        EXPECT_THROW({
            Socket client("::1", ipv4Server.getPort(), SocketType::Wifi, SocketProtocol::TCP);
        }, SocketException);
        
        // Make sure theres no incoming connections
        EXPECT_THROW({
            ipv4Server.accept(1000);
        }, TimeoutException);

        ipv4Server.close();
    }

    /**
     * Ensure that calls to ServerSocket.accept() with a provided timeout waits for atleast the provided timeout before throwing a TimeoutException.
    */
    TEST_F(ServerSocketTest, TestServerSocketAcceptTimeout)
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        EXPECT_THROW({
            Socket serverClient = serverSocket.accept(2000);
        }, TimeoutException);

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        ASSERT_GE(2, std::chrono::duration_cast<std::chrono::seconds>(end - start).count());
    }
}
