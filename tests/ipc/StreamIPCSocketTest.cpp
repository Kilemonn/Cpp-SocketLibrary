#include <string>

#include <gtest/gtest.h>

#include "../../src/ipc/IPCServerSocket.h"
#include "../../src/ipc/StreamIPCSocket.h"

#include "../../src/socketexceptions/SocketError.h"
#include "../../src/socketexceptions/SocketException.hpp"

const std::string SOCKET_PATH = "/tmp/StreamIPCSocketTest.sock";

namespace kt
{
    class StreamIPCSocketTest : public ::testing::Test
    {
    protected:
        IPCServerSocket serverSocket;
        StreamIPCSocket socket;

    protected:
        StreamIPCSocketTest() : serverSocket(SOCKET_PATH), socket(SOCKET_PATH) { }
        void TearDown() override
        {
            socket.close();
            serverSocket.close();
        }
    };

    TEST_F(StreamIPCSocketTest, IPCConstructor)
    {
        ASSERT_TRUE(socket.connected());
        ASSERT_FALSE(socket.ready());
        ASSERT_EQ(SOCKET_PATH, socket.getSocketPath());
        ASSERT_FALSE(kt::isInvalidSocket(socket.getSocket()));
    }

    /*
     * Ensure that a Socket created from the copy constructor is still able to send and receive from the copied socket.
     */
    TEST_F(StreamIPCSocketTest, IPCCopyConstructor)
    {
        StreamIPCSocket server = serverSocket.accept();
        StreamIPCSocket copiedSocket(socket);
        
        ASSERT_EQ(socket.getSocket(), copiedSocket.getSocket());
        ASSERT_EQ(socket.getSocketPath(), copiedSocket.getSocketPath());
        ASSERT_EQ(socket.connected(), copiedSocket.connected());

        const std::string testString = "Test";
        ASSERT_EQ(server.send(testString), testString.size());

        ASSERT_TRUE(socket.ready());
        ASSERT_TRUE(copiedSocket.ready());

        const std::string response = copiedSocket.receiveAmount(testString.size());
        ASSERT_EQ(response, testString);

        ASSERT_FALSE(socket.ready());
        ASSERT_FALSE(copiedSocket.ready());

        copiedSocket.close();
        ASSERT_FALSE(socket.connected());
        
        server.close();
    }

    /*
     * Ensure that a connected socket and accepted socket are correcly marked as connected.
     */
    TEST_F(StreamIPCSocketTest, IPCConnected)
    {
        StreamIPCSocket server = serverSocket.accept();
        ASSERT_TRUE(socket.connected());
        ASSERT_TRUE(server.connected());

        server.close();
    }

    TEST_F(StreamIPCSocketTest, IPCUnlinkTest)
    {
        serverSocket.close();
        ASSERT_THROW(StreamIPCSocket client(SOCKET_PATH), SocketException);
    }

    TEST_F(StreamIPCSocketTest, TCPReceiveAmount)
    {
        StreamIPCSocket server = serverSocket.accept();
        const std::string testString = "test";
        ASSERT_FALSE(server.ready());
        ASSERT_EQ(socket.send(testString), testString.size());
        ASSERT_TRUE(server.ready());
        std::string response = server.receiveAmount(testString.size());
        ASSERT_EQ(response, testString);

        server.close();
    }

    TEST_F(StreamIPCSocketTest, TCPReceiveAll)
    {
        StreamIPCSocket server = serverSocket.accept();
        const std::string testString = "test";
        ASSERT_FALSE(server.ready());
        ASSERT_EQ(socket.send(testString + testString + testString), testString.size() * 3);
        ASSERT_TRUE(server.ready());
        std::string response = server.receiveAll();
        ASSERT_EQ(response, testString + testString + testString);

        server.close();
    }

    TEST_F(StreamIPCSocketTest, TCPReceiveToDelimiter)
    {
        StreamIPCSocket server = serverSocket.accept();
        const std::string testString = "test";
        char delimiter = '&';
        ASSERT_FALSE(socket.ready());
        ASSERT_EQ(server.send(testString + testString + delimiter + testString), (testString.size() * 3) + 1);
        ASSERT_TRUE(socket.ready());
        std::string response = socket.receiveToDelimiter(delimiter);
        ASSERT_TRUE(socket.ready());
        ASSERT_EQ(response, testString + testString);

        server.close();
    }

    TEST_F(StreamIPCSocketTest, TCPReceiveToDelimiter_InvalidDelimiter)
    {
        ASSERT_THROW(socket.receiveToDelimiter('\0'), SocketException);
    }

    TEST_F(StreamIPCSocketTest, TCPGet)
    {
        StreamIPCSocket server = serverSocket.accept();
        const std::string testString = "test";
        ASSERT_FALSE(socket.ready());
        ASSERT_EQ(server.send(testString), testString.size());
        ASSERT_TRUE(socket.ready());
        std::optional<char> response = socket.get();
        ASSERT_EQ(*response, 't');

        ASSERT_TRUE(socket.ready());
        response = socket.get();
        ASSERT_EQ(*response, 'e');

        ASSERT_TRUE(socket.ready());
        response = socket.get();
        ASSERT_EQ(*response, 's');

        ASSERT_TRUE(socket.ready());
        response = socket.get();
        ASSERT_EQ(*response, 't');
        ASSERT_FALSE(socket.ready());

        server.close();
    }

    TEST_F(StreamIPCSocketTest, TCPClose)
    {
        StreamIPCSocket server = serverSocket.accept();
        ASSERT_TRUE(socket.connected());
        socket.close();
        ASSERT_FALSE(socket.connected());
        server.close();
    }
}
