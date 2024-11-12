#include <string>
#include <optional>
#include <climits>
#include <chrono>
#include <thread>
#include <csignal>

#include <gtest/gtest.h>

#include "../../src/socket/TCPSocket.h"
#include "../../src/serversocket/ServerSocket.h"
#include "../../src/socketexceptions/BindingException.hpp"
#include "../../src/socketexceptions/SocketError.h"

const std::string LOCALHOST = "localhost"; //"127.0.0.1";

namespace kt
{
    class TCPSocketTest : public ::testing::Test
    {
    protected:
        ServerSocket serverSocket;
        TCPSocket socket;

    protected:
        TCPSocketTest() : serverSocket(SocketType::Wifi), socket(LOCALHOST, serverSocket.getPort()) { }
        void TearDown() override
        {
            socket.close();
            serverSocket.close();
        }
    };

    /*
     * Ensure that the default construtor properties are set correct when the socket connects successfully.
     */
    TEST_F(TCPSocketTest, TCPConstructor)
    {
        ASSERT_TRUE(socket.connected());
        ASSERT_FALSE(socket.ready());
        ASSERT_EQ(LOCALHOST, socket.getHostname());
        ASSERT_EQ(serverSocket.getInternetProtocolVersion(), socket.getInternetProtocolVersion());
        ASSERT_FALSE(kt::isInvalidSocket(socket.getSocket()));
    }

    /*
     * Ensure that an empty hostname throws an exception.
     */
    TEST_F(TCPSocketTest, TCPConstructor_NoHostname)
    {
        ASSERT_THROW({
            TCPSocket emptyHostname("", serverSocket.getPort());
        }, SocketException);
    }
    
    /*
     * Ensure a SocketException is thrown when there is no listening server.
     */
    TEST_F(TCPSocketTest, TCPConstructor_NoListeningServerSocket)
    {
        serverSocket.close();
        socket.close();

        ASSERT_THROW({
            TCPSocket failedSocket(LOCALHOST, serverSocket.getPort());
        }, SocketException);
    }

    /*
     * Ensure a SocketException is thrown if you attempt to connect to a port where no server is listening.
     */
    TEST_F(TCPSocketTest, TCPConstructor_IncorrectPort)
    {
        ASSERT_THROW({
            TCPSocket failedSocket(LOCALHOST, serverSocket.getPort() + 1);
        }, SocketException);
    }

    // Ensure we can construct and connect to a server from the address it is listening on
    TEST_F(TCPSocketTest, TCPConstructor_FromAddress)
    {
        // Accept current incoming connection
        TCPSocket server = serverSocket.acceptTCPConnection();

        TCPSocket fromAddress(serverSocket.getSocketAddress());
        TCPSocket acceptedFromAddress = serverSocket.acceptTCPConnection();

        std::string sentFromAddress = "sentFromAddress";
        ASSERT_TRUE(fromAddress.send(sentFromAddress).first);
        ASSERT_TRUE(acceptedFromAddress.ready());
        ASSERT_EQ(sentFromAddress, acceptedFromAddress.receiveAmount(sentFromAddress.size()));

        std::string sentFromServer = "sentFromServer";
        ASSERT_TRUE(acceptedFromAddress.send(sentFromServer).first);
        ASSERT_TRUE(fromAddress.ready());
        ASSERT_EQ(sentFromServer, fromAddress.receiveAmount(sentFromServer.size()));
    }

    // Ensure we throw a SocketException if we cannot construct a TCP socket from the provided SocketAddress
    TEST_F(TCPSocketTest, TCPConstructor_FromEmptyAddress)
    {
        // Accept the incoming connection to make sure the server is ready
        TCPSocket server = serverSocket.acceptTCPConnection();

        kt::SocketAddress address{};
        ASSERT_THROW(TCPSocket fromEmptyAddress(address), SocketException);
    }
    
    /*
     * Ensure that a Socket created from the copy constructor is still able to send and receive from the copied socket.
     */
    TEST_F(TCPSocketTest, TCPCopyConstructor)
    {
        TCPSocket server = serverSocket.acceptTCPConnection();
        TCPSocket copiedSocket(socket);
        
        ASSERT_EQ(socket.getSocket(), copiedSocket.getSocket());
        ASSERT_EQ(socket.getHostname(), copiedSocket.getHostname());
        ASSERT_EQ(socket.getPort(), copiedSocket.getPort());
        ASSERT_EQ(socket.getInternetProtocolVersion(), copiedSocket.getInternetProtocolVersion());
        kt::SocketAddress initialAddress = socket.getSocketAddress();
        kt::SocketAddress copiedAddress = copiedSocket.getSocketAddress();
        ASSERT_EQ(0, std::memcmp(&initialAddress, &copiedAddress, sizeof(initialAddress)));

        const std::string testString = "Test";
        ASSERT_TRUE(copiedSocket.send(testString).first);
        const std::string response = server.receiveAmount(testString.size());
        ASSERT_EQ(response, testString);

        copiedSocket.close();
        server.close();
    }

    /*
     * Ensure that a connected socket and accepted socket are correcly marked as connected.
     */
    TEST_F(TCPSocketTest, TCPConnected)
    {
        TCPSocket server = serverSocket.acceptTCPConnection();
        ASSERT_TRUE(socket.connected());
        ASSERT_TRUE(server.connected());

        server.close();
    }

    TEST_F(TCPSocketTest, TCPReceiveAmount)
    {
        TCPSocket server = serverSocket.acceptTCPConnection();
        const std::string testString = "test";
        ASSERT_FALSE(server.ready());
        ASSERT_TRUE(socket.send(testString).first);
        ASSERT_TRUE(server.ready());
        std::string response = server.receiveAmount(testString.size());
        ASSERT_EQ(response, testString);

        server.close();
    }

    TEST_F(TCPSocketTest, TCPReceiveAll)
    {
        TCPSocket server = serverSocket.acceptTCPConnection();
        const std::string testString = "test";
        ASSERT_FALSE(server.ready());
        ASSERT_TRUE(socket.send(testString + testString + testString).first);
        ASSERT_TRUE(server.ready());
        std::string response = server.receiveAll();
        ASSERT_EQ(response, testString + testString + testString);

        server.close();
    }

    TEST_F(TCPSocketTest, TCPReceiveToDelimiter)
    {
        TCPSocket server = serverSocket.acceptTCPConnection();
        const std::string testString = "test";
        char delimiter = '&';
        ASSERT_FALSE(socket.ready());
        ASSERT_TRUE(server.send(testString + testString + delimiter + testString).first);
        ASSERT_TRUE(socket.ready());
        std::string response = socket.receiveToDelimiter(delimiter);
        ASSERT_TRUE(socket.ready());
        ASSERT_EQ(response, testString + testString);

        server.close();
    }

    TEST_F(TCPSocketTest, TCPReceiveToDelimiter_InvalidDelimiter)
    {
        ASSERT_THROW(socket.receiveToDelimiter('\0'), SocketException);
    }

    TEST_F(TCPSocketTest, TCPGet)
    {
        TCPSocket server = serverSocket.acceptTCPConnection();
        const std::string testString = "test";
        ASSERT_FALSE(socket.ready());
        ASSERT_TRUE(server.send(testString).first);
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

    TEST_F(TCPSocketTest, TCPClose)
    {
        TCPSocket server = serverSocket.acceptTCPConnection();
        ASSERT_TRUE(socket.connected());
        socket.close();
        ASSERT_FALSE(socket.connected());
        server.close();
    }

#ifdef __linux__
    bool sigPipeHandlerWasCalled = false;
    void handleSignal(int signal)
    {
        if (signal == SIGPIPE)
        {
            sigPipeHandlerWasCalled = true;
        }
    }

    TEST_F(TCPSocketTest, TestLinuxSendToClosedSocket_SIGPIPE_CustomHandlerFunction)
    {
        ASSERT_NE(std::signal(SIGPIPE, handleSignal), SIG_ERR);

        TCPSocket server = serverSocket.acceptTCPConnection();
        server.close();

        ASSERT_FALSE(sigPipeHandlerWasCalled);

        const std::string message = "TestLinuxSendToClosedSocket_SIGPIPE_CustomHandlerFunction";
        // The first send does not detect the disconnection?
        std::pair<bool, int> result = socket.send(message);
        ASSERT_TRUE(result.first);
        
        result = socket.send(message);
        ASSERT_FALSE(result.first);

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
        ASSERT_TRUE(sigPipeHandlerWasCalled);

        // Revert the sigpipe handler
        ASSERT_NE(std::signal(SIGPIPE, SIG_DFL), SIG_ERR);
    }

    TEST_F(TCPSocketTest, TestLinuxSendToClosedSocket_SIGPIPE_MSG_NOSIGNALFlag)
    {
        TCPSocket server = serverSocket.acceptTCPConnection();
        server.close();

        const std::string message = "TestLinuxSendToClosedSocket_SIGPIPE_MSG_NOSIGNALFlag";
        // Make sure you call the correct override of the method
        std::pair<bool, int> result = socket.send(message, MSG_NOSIGNAL);
        ASSERT_TRUE(result.first);
        
        result = socket.send(message, MSG_NOSIGNAL);
        ASSERT_FALSE(result.first);
    }

    TEST_F(TCPSocketTest, TestLinuxSendToClosedSocket_SIGPIPE_IgnoreSignals)
    {
        // Ignore SIGPIPE signals
        ASSERT_NE(std::signal(SIGPIPE, SIG_IGN), SIG_ERR);

        TCPSocket server = serverSocket.acceptTCPConnection();
        server.close();

        const std::string message = "TestLinuxSendToClosedSocket_SIGPIPE_IgnoreSignals";
        // Make sure you call the correct override of the method
        std::pair<bool, int> result = socket.send(message, MSG_NOSIGNAL);
        ASSERT_TRUE(result.first);
        
        result = socket.send(message, MSG_NOSIGNAL);
        ASSERT_FALSE(result.first);

        // Revert behaviour for other tests
        ASSERT_NE(std::signal(SIGPIPE, SIG_DFL), SIG_ERR);
    }

#endif

    TEST_F(TCPSocketTest, IPV6Address)
    {
        ServerSocket ipv6ServerSocket(SocketType::Wifi, std::nullopt, 0, 20, InternetProtocolVersion::IPV6);
        
        TCPSocket ipv6Socket("0:0:0:0:0:0:0:1", ipv6ServerSocket.getPort());

        // Accept ipv6 connnection
        TCPSocket ipv6Server = ipv6ServerSocket.acceptTCPConnection();
        ASSERT_TRUE(ipv6Server.connected());

        const std::string testString = "Test";
        ASSERT_TRUE(ipv6Socket.send(testString).first);
        const std::string response = ipv6Server.receiveAmount(testString.size());
        ASSERT_EQ(response, testString);

        ipv6Server.close();
        ipv6Socket.close();
        ipv6ServerSocket.close();
    }
    
    /**
     * Sending a payload larger than the send buffer capacity to determine the behaviour and limit.
     */
    TEST_F(TCPSocketTest, LargePayloadSendAndRecv)
    {
        TCPSocket server = serverSocket.acceptTCPConnection();

        int receiveBufferSize = 0;
        socklen_t size = sizeof(receiveBufferSize);
        int result = getsockopt(socket.getSocket(), SOL_SOCKET, SO_RCVBUF, (char*)&receiveBufferSize, &size);
        if (result == -1)
        {
            std::cout << "Unable to get recv buffer size, skipping test." << std::endl;
            GTEST_SKIP();
            return;
        }

        int sendBufferSize = 0;
        size = sizeof(sendBufferSize);
        result = getsockopt(socket.getSocket(), SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, &size);
        if (result == -1)
        {
            std::cout << "Unable to get send buffer size, skipping test." << std::endl;
            GTEST_SKIP();
            return;
        }

#ifdef _WIN32
        // From my testing Windows must do some additional buffering and has no integer limit to how much it sends and receives via TCP.
        int upperBound = INT_MAX; // sendBufferSize * 1000;
#elif __linux__

        // Sending a string that is 98.5+% the size of the send buffer will cause the send to hang
        // So we will send a string the size of 98.4% the size of the buffer limit
        int upperBound = sendBufferSize * 0.984;
#endif

        std::string message(upperBound, 'c');
        ASSERT_GT(message.size(), receiveBufferSize);

        std::pair<bool, int> sendResult = socket.send(message);
        
        ASSERT_EQ(message.size(), sendResult.second);
        ASSERT_TRUE(server.ready());

        std::string recieved = server.receiveAmount(upperBound);
        ASSERT_EQ(message, recieved);
    }
}
