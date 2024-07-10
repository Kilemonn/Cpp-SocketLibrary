
#include <string>
#include <optional>

#include <gtest/gtest.h>

#include "../../src/socket/UDPSocket.h"
#include "../../src/socketexceptions/BindingException.hpp"

const std::string LOCALHOST = "localhost";

namespace kt
{
    class UDPSocketTest : public ::testing::Test
    {
    protected:
        UDPSocket socket;

    protected:
        UDPSocketTest() : socket() { }
        void TearDown() override
        {
            this->socket.close();
        }
    };

    /*
     * Test the kt::UDPSocket constructors and exception handling for UDP. This covers the following scenarios:
     * - Constructing a socket and ensuring its default values are set correctly
     */
    TEST_F(UDPSocketTest, UDPConstructors)
    {
        ASSERT_FALSE(socket.ready());
        ASSERT_FALSE(socket.isUdpBound());
        ASSERT_EQ(std::nullopt, socket.getListeningPort());
        ASSERT_EQ(kt::InternetProtocolVersion::Any, socket.getInternetProtocolVersion());
        ASSERT_TRUE(kt::isInvalidSocket(socket.getListeningSocket()));
    }

    TEST_F(UDPSocketTest, UDPCopyConstructors)
    {
        ASSERT_TRUE(socket.bind().first);
        UDPSocket copiedSocket(socket);

        ASSERT_EQ(socket.getListeningSocket(), copiedSocket.getListeningSocket());
        ASSERT_EQ(socket.isUdpBound(), copiedSocket.isUdpBound());
        ASSERT_NE(std::nullopt, socket.getListeningPort());
        ASSERT_NE(std::nullopt, copiedSocket.getListeningPort());
        ASSERT_EQ(socket.getListeningPort().value(), copiedSocket.getListeningPort().value());
        ASSERT_EQ(socket.getInternetProtocolVersion(), copiedSocket.getInternetProtocolVersion());

        copiedSocket.close();
    }

    /*
     * Ensure that multiple calls to UDPSocket.bind() fails if another socket is already bound to that port.
     */
    TEST_F(UDPSocketTest, UDPBindAndBound_MultipleCalls)
    {
        ASSERT_FALSE(socket.isUdpBound());
        ASSERT_EQ(kt::InternetProtocolVersion::Any, socket.getInternetProtocolVersion());
        ASSERT_TRUE(socket.bind().first);
        ASSERT_NE(kt::InternetProtocolVersion::Any, socket.getInternetProtocolVersion());
        ASSERT_TRUE(socket.isUdpBound());

        kt::UDPSocket newServer;
        ASSERT_FALSE(newServer.isUdpBound());
        ASSERT_NE(std::nullopt, socket.getListeningPort());
        EXPECT_THROW(newServer.bind(socket.getListeningPort().value()), BindingException);
    }

    /*
     * Ensure that calling UDPSocket.bind() with 0 as the port will resolve to another port from the OS.
     */
    TEST_F(UDPSocketTest, UDPBind_WithoutSpecifiedPort)
    {
        ASSERT_FALSE(socket.isUdpBound());
        ASSERT_TRUE(socket.bind(0).first);
        ASSERT_TRUE(socket.isUdpBound());
        ASSERT_NE(0, socket.getListeningPort());
    }

    /*
     * Test UDPSocket.sendTo() to ensure that it can send correctly to the listening socket.
     */
    TEST_F(UDPSocketTest, UDPSendTo)
    {
        ASSERT_TRUE(socket.bind().first);

        UDPSocket client;

        ASSERT_FALSE(socket.ready());
        const std::string testString = "test";
        ASSERT_TRUE(client.sendTo(LOCALHOST, socket.getListeningPort().value(), testString).first.first);
        ASSERT_TRUE(socket.ready());
    }

    /**
     * Ensure that sending with an empty hostname failed to resolve the address and fail.
     */
    TEST_F(UDPSocketTest, TestEmptyHostname)
    {
        ASSERT_TRUE(socket.bind().first);
        
        UDPSocket client;
        ASSERT_FALSE(socket.ready());
        const std::string message = "test";
        ASSERT_FALSE(client.sendTo("", socket.getListeningPort().value(), message, 0, socket.getInternetProtocolVersion()).first.first);
        ASSERT_FALSE(socket.ready());
    }

    /*
     * Call UDPSocket.receiveFrom() to make sure the correct amount of data is read.
     */
    TEST_F(UDPSocketTest, UDPReceiveFrom)
    {
        ASSERT_TRUE(socket.bind().first);
        ASSERT_FALSE(socket.ready());

        UDPSocket client;
        const std::string testString = "test";
        ASSERT_TRUE(client.sendTo(LOCALHOST, socket.getListeningPort().value(), testString).first.first);

        ASSERT_TRUE(socket.ready());
        std::pair<std::optional<std::string>, std::pair<int, kt::SocketAddress>> recieved = socket.receiveFrom(testString.size());
        ASSERT_FALSE(socket.ready());
        ASSERT_NE(std::nullopt, recieved.first);
        ASSERT_EQ(testString, recieved.first.value());
    }

    /**
     * Ensure that receiveAmount reads the specified amount even when more is available in the buffer.
     * Also confirm that the remaining data is lost if not read.
     */
    TEST_F(UDPSocketTest, UDPReceiveAmount_NotEnoughRead)
    {
        ASSERT_TRUE(socket.bind().first);
        ASSERT_FALSE(socket.ready());

        UDPSocket client;
        const std::string testString = "test";
        ASSERT_TRUE(client.sendTo(LOCALHOST, socket.getListeningPort().value(), testString).first.first);

        ASSERT_TRUE(socket.ready());
        std::pair<std::optional<std::string>, std::pair<int, kt::SocketAddress>> recieved = socket.receiveFrom(testString.size() - 1);
        ASSERT_FALSE(socket.ready());
        ASSERT_NE(std::nullopt, recieved.first);
        ASSERT_EQ(testString.substr(0, testString.size() - 1), recieved.first.value());
    }

    /*
     * Ensure that the receiveAmount reads the correct amount when less data is provided than expected.
     */
    TEST_F(UDPSocketTest, UDPReceiveAmount_TooMuchRead)
    {
        ASSERT_TRUE(socket.bind().first);
        ASSERT_FALSE(socket.ready());

        UDPSocket client;
        const std::string testString = "test";
        ASSERT_TRUE(client.sendTo(LOCALHOST, socket.getListeningPort().value(), testString).first.first);

        ASSERT_TRUE(socket.ready());
        std::pair<std::optional<std::string>, std::pair<int, kt::SocketAddress>> recieved = socket.receiveFrom(testString.size() + 1);
        ASSERT_FALSE(socket.ready());
        ASSERT_NE(std::nullopt, recieved.first);
        ASSERT_EQ(testString, recieved.first.value());
    }

    /**
     * Ensure that sendTo() returns the send address properly and that it can be used in future calls to skip the address resolution step.
     */
    TEST_F(UDPSocketTest, UDPSendToAddress)
    {
        ASSERT_TRUE(socket.bind().first);
        ASSERT_FALSE(socket.ready());

        UDPSocket client;
        std::string testString = "test";
        std::pair<std::pair<bool, int>, kt::SocketAddress> sendResult = client.sendTo(LOCALHOST, socket.getListeningPort().value(), testString);
        ASSERT_TRUE(sendResult.first.first);

        ASSERT_TRUE(socket.ready());
        std::pair<std::optional<std::string>, std::pair<int, kt::SocketAddress>> recieved = socket.receiveFrom(testString.size());
        ASSERT_FALSE(socket.ready());
        ASSERT_NE(std::nullopt, recieved.first);
        ASSERT_EQ(testString, recieved.first.value());

        testString += testString + testString;
        // Now send using the address resolved and returned from the first call to sendTo()
        ASSERT_TRUE(client.sendTo(testString, sendResult.second).first);
        ASSERT_TRUE(socket.ready());
        recieved = socket.receiveFrom(testString.size());
        ASSERT_FALSE(socket.ready());
        ASSERT_NE(std::nullopt, recieved.first);
        ASSERT_EQ(testString, recieved.first.value());
    }

    /**
     * This test is focused on prooving that you can re-use an existing address that you received data from and reply as long as you set the recieving port correctly.
     * 
     * Write test to re-use the received address + new port to send back to the senders
     */
    TEST_F(UDPSocketTest, UDPManipulateAddress_IPV4)
    {
        ASSERT_TRUE(socket.bind(0, kt::InternetProtocolVersion::IPV4).first);

        kt::UDPSocket client;
        ASSERT_TRUE(client.bind(0, kt::InternetProtocolVersion::IPV4).first);

        std::string message = std::to_string(client.getListeningPort().value());
        ASSERT_TRUE(client.sendTo("127.0.0.1", socket.getListeningPort().value(), message).first.first);

        std::pair<std::optional<std::string>, std::pair<int, kt::SocketAddress>> result = socket.receiveFrom(50);
        ASSERT_NE(result.second.first, -1);
        kt::SocketAddress address = result.second.second;

        message = "UDPManipulateAddress";
        address.ipv4.sin_port = htons(client.getListeningPort().value());

        ASSERT_TRUE(socket.sendTo(message, address).first);

        result = client.receiveFrom(message.size());
        ASSERT_EQ(message, result.first.value());
    }

    TEST_F(UDPSocketTest, UDPManipulateAddress_IPV6)
    {
        ASSERT_TRUE(socket.bind(0, kt::InternetProtocolVersion::IPV6).first);

        kt::UDPSocket client;
        ASSERT_TRUE(client.bind(0, kt::InternetProtocolVersion::IPV6).first);

        std::string message = std::to_string(client.getListeningPort().value());
        ASSERT_TRUE(client.sendTo("::1", socket.getListeningPort().value(), message).first.first);

        std::pair<std::optional<std::string>, std::pair<int, kt::SocketAddress>> result = socket.receiveFrom(50);
        ASSERT_NE(result.second.first, -1);
        kt::SocketAddress address = result.second.second;

        message = "UDPManipulateAddress";
        // We can set the ipv4 address since its in the same position and data type in both ipv4 address and ipv6 address
        address.ipv4.sin_port = htons(client.getListeningPort().value());

        ASSERT_TRUE(socket.sendTo(message, address).first);

        result = client.receiveFrom(message.size());
        ASSERT_EQ(message, result.first.value());
    }

    // Use the returned .bind() address to use as the socket address that is used to send a message
    TEST_F(UDPSocketTest, SendToBoundAddress)
    {
        // Setting IP version here for windows as its seems to be biasing towards different IP versions
        std::pair<bool, kt::SocketAddress> bindResult = socket.bind(0, kt::InternetProtocolVersion::IPV4);
        ASSERT_TRUE(bindResult.first);

        UDPSocket client;
        ASSERT_FALSE(socket.ready());
        const std::string message = "SendToBoundAddress";
        ASSERT_TRUE(client.sendTo(message, bindResult.second).first);
        ASSERT_TRUE(socket.ready());
    }

    TEST_F(UDPSocketTest, LargePayloadSend)
    {
        std::pair<bool, kt::SocketAddress> bindResult = socket.bind(0, kt::InternetProtocolVersion::IPV4);
        ASSERT_TRUE(bindResult.first);

        int receiveBufferSize;
        socklen_t size = sizeof(receiveBufferSize);
        int result = getsockopt(socket.getListeningSocket(), SOL_SOCKET, SO_RCVBUF, (char*)&receiveBufferSize, &size);
        if (result == -1)
        {
            std::cout << "Unable to get recv buffer size, skipping test." << std::endl;
            GTEST_SKIP();
            return;
        }

        int sendBufferSize;
        size = sizeof(sendBufferSize);
        result = getsockopt(socket.getListeningSocket(), SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, &size);
        if (result == -1)
        {
            std::cout << "Unable to get send buffer size, skipping test." << std::endl;
            GTEST_SKIP();
            return;
        }

        kt::UDPSocket client;
        ASSERT_EQ(sendBufferSize, receiveBufferSize);

        // Creating a message 2 times bigger than the send size
        std::string message(sendBufferSize * 2, 'c');
        std::pair<std::pair<bool, int>, kt::SocketAddress> sendResult = client.sendTo("127.0.0.1", socket.getListeningPort().value(), message);
        
        ASSERT_EQ(-1, sendResult.first.second);
        ASSERT_FALSE(socket.ready());
    }

    TEST_F(UDPSocketTest, LargePayloadRecieve_AndPreSendSocketOperation)
    {
        std::pair<bool, kt::SocketAddress> bindResult = socket.bind(0, kt::InternetProtocolVersion::IPV4);
        ASSERT_TRUE(bindResult.first);

        int receiveBufferSize;
        socklen_t size = sizeof(receiveBufferSize);
        int result = getsockopt(socket.getListeningSocket(), SOL_SOCKET, SO_RCVBUF, (char*)&receiveBufferSize, &size);
        if (result == -1)
        {
            std::cout << "Unable to get recv buffer size, skipping test." << std::endl;
            GTEST_SKIP();
            return;
        }

        int sendBufferSize;
        size = sizeof(sendBufferSize);
        result = getsockopt(socket.getListeningSocket(), SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, &size);
        if (result == -1)
        {
            std::cout << "Unable to get send buffer size, skipping test." << std::endl;
            GTEST_SKIP();
            return;
        }

        kt::UDPSocket sender;
        int initialSendBufferSize = sendBufferSize;
        ASSERT_EQ(sendBufferSize, receiveBufferSize);
        sender.setPreSendSocketOperation([&sendBufferSize](SOCKET& sendSocket)
        {
            int doubledSendBufferSize = sendBufferSize * 2;
            socklen_t size = sizeof(doubledSendBufferSize);
            int result = setsockopt(sendSocket, SOL_SOCKET, SO_SNDBUF, (char*)&doubledSendBufferSize, size);
            if (result == -1)
            {
                std::cout << "Returning early from LargePayloadSend, since we can't set the buffer size" << std::endl;
                return;
            }

            int updatedBufferSize = sendBufferSize;
            result = getsockopt(sendSocket, SOL_SOCKET, SO_SNDBUF, (char*)&updatedBufferSize, &size);

            if (updatedBufferSize <= sendBufferSize)
            {
                // Buffer size unchanged so we can leave this test
                std::cout << "Unable to change send buffer size [" << sendBufferSize << "]." << std::endl;
                return;
            }

            sendBufferSize = updatedBufferSize;
        });

        std::string message(initialSendBufferSize, 'c');
        std::pair<std::pair<bool, int>, kt::SocketAddress> sendResult = sender.sendTo("127.0.0.1", socket.getListeningPort().value(), message);

        if (sendBufferSize > initialSendBufferSize)
        {
            ASSERT_GT(sendBufferSize, receiveBufferSize);

            ASSERT_EQ(-1, sendResult.first.second);
            ASSERT_FALSE(socket.ready());
        }
        else
        {
            std::cout << "Unable to increase buffer size, skipping test." << std::endl;
            GTEST_SKIP();
        }
    }
}
