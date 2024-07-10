
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
        ASSERT_TRUE(client.sendTo(LOCALHOST, socket.getListeningPort().value(), testString).first);
        ASSERT_TRUE(socket.ready());
    }

    /*
     * 
     */
    TEST_F(UDPSocketTest, TestEmptyHostname_IPV4)
    {
        ASSERT_TRUE(socket.bind(0, kt::InternetProtocolVersion::IPV4).first);
        
        UDPSocket client;
        ASSERT_FALSE(socket.ready());
        const std::string message = "test";
        ASSERT_TRUE(client.sendTo("", socket.getListeningPort().value(), message, 0, kt::InternetProtocolVersion::IPV4).first);
        ASSERT_TRUE(socket.ready());
    }

    /*
     * 
     */
    TEST_F(UDPSocketTest, TestEmptyHostname_IPV6)
    {
        ASSERT_TRUE(socket.bind(0, kt::InternetProtocolVersion::IPV6).first);
        
        UDPSocket client;
        ASSERT_FALSE(socket.ready());
        const std::string message = "test";
        ASSERT_TRUE(client.sendTo("", socket.getListeningPort().value(), message, 0, kt::InternetProtocolVersion::IPV6).first);
        ASSERT_TRUE(socket.ready());
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
        ASSERT_TRUE(client.sendTo(LOCALHOST, socket.getListeningPort().value(), testString).first);

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
        ASSERT_TRUE(client.sendTo(LOCALHOST, socket.getListeningPort().value(), testString).first);

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
        ASSERT_TRUE(client.sendTo(LOCALHOST, socket.getListeningPort().value(), testString).first);

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
        std::pair<bool, std::pair<int, kt::SocketAddress>> sendResult = client.sendTo(LOCALHOST, socket.getListeningPort().value(), testString);
        ASSERT_TRUE(sendResult.first);

        ASSERT_TRUE(socket.ready());
        std::pair<std::optional<std::string>, std::pair<int, kt::SocketAddress>> recieved = socket.receiveFrom(testString.size());
        ASSERT_FALSE(socket.ready());
        ASSERT_NE(std::nullopt, recieved.first);
        ASSERT_EQ(testString, recieved.first.value());

        testString += testString + testString;
        // Now send using the address resolved and returned from the first call to sendTo()
        ASSERT_TRUE(client.sendTo(testString, sendResult.second.second).first);
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
        ASSERT_TRUE(client.sendTo("127.0.0.1", socket.getListeningPort().value(), message).first);

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

    }

    // TODO: large payload tests
}
