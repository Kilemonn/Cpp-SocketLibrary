#include <string>
#include <optional>

#include <gtest/gtest.h>

#include "../../src/socket/Socket.h"
#include "../../src/serversocket/ServerSocket.h"
#include "../../src/socketexceptions/BindingException.hpp"

const std::string LOCALHOST = "127.0.0.1";

namespace kt
{
    class SocketTCPTest : public ::testing::Test
    {
    protected:
        ServerSocket serverSocket;
        Socket socket;

    protected:
        SocketTCPTest() : serverSocket(SocketType::Wifi, 0), socket(LOCALHOST, serverSocket.getPort(), SocketType::Wifi, SocketProtocol::TCP) { }
        void TearDown() override
        {
            socket.close();
            serverSocket.close();
        }
    };

    /*
     * Ensure that the default construtor properties are set correct when the socket connects successfully.
     */
    TEST_F(SocketTCPTest, TCPConstructor)
    {
        ASSERT_FALSE(socket.isBound());
        ASSERT_TRUE(socket.connected());
        ASSERT_EQ(socket.getType(), SocketType::Wifi);
        ASSERT_EQ(socket.getProtocol(), SocketProtocol::TCP);
        ASSERT_FALSE(socket.ready());
        ASSERT_EQ(LOCALHOST, socket.getAddress());
        ASSERT_EQ(std::nullopt, socket.getLastRecievedAddress());
    }

    /*
     * Ensure that a SocketException is thrown when a Wifi socket with a protocol set to None is created.
     */
    TEST_F(SocketTCPTest, TCPConstructor_NoProtocol)
    {
        ASSERT_THROW({
            Socket failedSocket(LOCALHOST, serverSocket.getPort(), SocketType::Wifi);
        }, SocketException);
    }

    /*
     * Ensure that a SocketException is thrown when there is no hostname provided.
     */
    TEST_F(SocketTCPTest, TCPConstructor_NoHostname)
    {
        ASSERT_THROW({
            Socket failedSocket("", serverSocket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::TCP);
        }, SocketException);
    }
    
    /*
     * Ensure a SocketException is thrown when there is no listening server.
     */
    TEST_F(SocketTCPTest, TCPConstructor_NoListeningServerSocket)
    {
        serverSocket.close();
        socket.close();

        ASSERT_THROW({
            Socket failedSocket(LOCALHOST, serverSocket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::TCP);
        }, SocketException);
    }

    /*
     * Ensure a SocketException is thrown if you attempt to connect to a port where no server is listening.
     */
    TEST_F(SocketTCPTest, TCPConstructor_IncorrectPort)
    {
        ASSERT_THROW({
            Socket failedSocket(LOCALHOST, serverSocket.getPort() + 1, kt::SocketType::Wifi, kt::SocketProtocol::TCP);
        }, SocketException);
    }
    
    /*
     * Ensure that a Socket created from the copy constructor is still able to send and receive from the copied socket.
     */
    TEST_F(SocketTCPTest, TCPCopyConstructor)
    {
        Socket server = serverSocket.accept();
        Socket copiedSocket(socket);

        const std::string testString = "Test";
        ASSERT_TRUE(copiedSocket.send(testString));
        const std::string response = server.receiveAmount(testString.size());
        ASSERT_EQ(response, testString);

        copiedSocket.close();
        server.close();
    }

    /*
     * Ensure that a connected socket and accepted socket are correcly marked as connected.
     */
    TEST_F(SocketTCPTest, TCPConnected)
    {
        Socket server = serverSocket.accept();
        ASSERT_TRUE(socket.connected());
        ASSERT_TRUE(server.connected());

        server.close();
    }

    TEST_F(SocketTCPTest, TCPReceiveAmount)
    {
        Socket server = serverSocket.accept();
        const std::string testString = "test";
        ASSERT_FALSE(server.ready());
        ASSERT_TRUE(socket.send(testString));
        ASSERT_TRUE(server.ready());
        std::string response = server.receiveAmount(testString.size());
        ASSERT_EQ(response, testString);

        server.close();
    }

    TEST_F(SocketTCPTest, TCPReceiveAll)
    {
        Socket server = serverSocket.accept();
        const std::string testString = "test";
        ASSERT_FALSE(server.ready());
        ASSERT_TRUE(socket.send(testString + testString + testString));
        ASSERT_TRUE(server.ready());
        std::string response = server.receiveAll();
        ASSERT_EQ(response, testString + testString + testString);

        server.close();
    }

    TEST_F(SocketTCPTest, TCPReceiveToDelimiter)
    {
        Socket server = serverSocket.accept();
        const std::string testString = "test";
        char delimiter = '&';
        ASSERT_FALSE(socket.ready());
        ASSERT_TRUE(server.send(testString + testString + delimiter + testString));
        ASSERT_TRUE(socket.ready());
        std::string response = socket.receiveToDelimiter(delimiter);
        ASSERT_TRUE(socket.ready());
        ASSERT_EQ(response, testString + testString);

        server.close();
    }

    TEST_F(SocketTCPTest, TCPReceiveToDelimiter_InvalidDelimiter)
    {
        ASSERT_THROW(socket.receiveToDelimiter('\0'), SocketException);
    }

    TEST_F(SocketTCPTest, TCPGet)
    {
        Socket server = serverSocket.accept();
        const std::string testString = "test";
        ASSERT_FALSE(socket.ready());
        ASSERT_TRUE(server.send(testString));
        ASSERT_TRUE(socket.ready());
        char response = socket.get();
        ASSERT_EQ(response, 't');

        ASSERT_TRUE(socket.ready());
        response = socket.get();
        ASSERT_EQ(response, 'e');

        ASSERT_TRUE(socket.ready());
        response = socket.get();
        ASSERT_EQ(response, 's');

        ASSERT_TRUE(socket.ready());
        response = socket.get();
        ASSERT_EQ(response, 't');
        ASSERT_FALSE(socket.ready());

        server.close();
    }

    TEST_F(SocketTCPTest, TCPClose)
    {
        socket.close();
        ASSERT_FALSE(socket.connected());
    }
    
}
