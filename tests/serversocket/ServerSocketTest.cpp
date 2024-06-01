
#include <gtest/gtest.h>

#include "../../src/serversocket/ServerSocket.h"
#include "../../src/socketexceptions/BindingException.hpp"

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

        Socket client("localhost", serverSocket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::TCP);

        Socket serverClient = server2.accept();
        const std::string testString = "test";

        ASSERT_TRUE(client.send(testString));
        const std::string responseString = serverClient.receiveAmount(testString.size());
        ASSERT_EQ(responseString, testString);

        serverClient.close();
        client.close();
        server2.close();
    }
}

