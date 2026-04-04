#include <gtest/gtest.h>

#include "../../src/ipc/IPCServerSocket.h"
#include "../../src/socketexceptions/SocketError.h"
#include "../../src/socketexceptions/BindingException.hpp"
#include "../../src/socketexceptions/TimeoutException.hpp"

const std::string SOCKET_PATH = "/tmp/IPCServerSocketTest";

namespace kt
{
    class IPCServerSocketTest: public ::testing::Test
    {
    protected:
        IPCServerSocket serverSocket;
    protected:
        IPCServerSocketTest() : serverSocket(SOCKET_PATH) {}
        // void SetUp() override { }
        void TearDown() override
        {
            serverSocket.close();
            IPCSocket::closePath(SOCKET_PATH);
        }
    };

    /*
     * Ensure the defaults are set correctly.
     */
    TEST_F(IPCServerSocketTest, TestDefaultConstructor)
    {
        ASSERT_FALSE(kt::isInvalidSocket(serverSocket.getSocket()));
    }

    /*
     * Ensure thatt if a copied serversocket is closed, that it closes the copied socket too since they shared the same underlying descriptor.
     */
    TEST_F(IPCServerSocketTest, TestCloseCopiedServerSocket)
    {
        IPCServerSocket copiedServer = serverSocket;
        serverSocket.close();
        ASSERT_THROW(copiedServer.accept(), SocketException);
    }

    /*
     * Ensure the copy constructed server socket is able to connect to the client and receive messages.
     */
    TEST_F(IPCServerSocketTest, TestCopyConstructor)
    {
        IPCServerSocket server2(serverSocket);
        ASSERT_EQ(serverSocket.getSocket(), server2.getSocket());
        ASSERT_EQ(serverSocket.getSocketPath(), server2.getSocketPath());

        IPCSocket client(SOCKET_PATH);

        IPCSocket serverClient = server2.accept();
        const std::string testString = "test";

        ASSERT_EQ(client.send(testString), testString.size());
        const std::string responseString = serverClient.receiveAmount(testString.size());
        ASSERT_EQ(responseString, testString);

        serverClient.close();
        client.close();
        server2.close();
    }

    /*
     * Ensure that we throw a binding exception if the socket path is already being used/already exists.
     */
    TEST_F(IPCServerSocketTest, TestSocketPathAlreadyUsed)
    {
        EXPECT_THROW(IPCServerSocket server2(SOCKET_PATH), BindingException);
        IPCServerSocket server2(SOCKET_PATH, true);
        server2.close();
    }

    /**
     * Ensure that calls to ServerSocket.accept() with a provided timeout waits for atleast the provided timeout before throwing a TimeoutException.
    */
    TEST_F(IPCServerSocketTest, TestServerSocketAcceptTimeout)
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        EXPECT_THROW({
            IPCSocket serverClient = serverSocket.accept(1 * 1000000);
        }, TimeoutException);

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        ASSERT_GE(1, std::chrono::duration_cast<std::chrono::seconds>(end - start).count());
    }
}
