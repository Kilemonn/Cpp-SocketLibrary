
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
        ASSERT_EQ(0, socket.getListeningPort());
        ASSERT_EQ(kt::InternetProtocolVersion::Any, socket.getInternetProtocolVersion());
    }

    /*
     * Ensure that multiple calls to UDPSocket.bind() fails if another socket is already bound to that port.
     */
    TEST_F(UDPSocketTest, UDPBindAndBound_MultipleCalls)
    {
        ASSERT_FALSE(socket.isUdpBound());
        ASSERT_EQ(kt::InternetProtocolVersion::Any, socket.getInternetProtocolVersion());
        ASSERT_TRUE(socket.bind(0));
        ASSERT_NE(kt::InternetProtocolVersion::Any, socket.getInternetProtocolVersion());
        ASSERT_TRUE(socket.isUdpBound());

        kt::UDPSocket newServer;
        ASSERT_FALSE(newServer.isUdpBound());
        EXPECT_THROW(newServer.bind(socket.getListeningPort()), BindingException);
    }

    /*
     * Ensure that calling UDPSocket.bind() with 0 as the port will resolve to another port from the OS.
     */
    TEST_F(UDPSocketTest, UDPBind_WithoutSpecifiedPort)
    {
        const unsigned int port = 0;

        ASSERT_FALSE(socket.isUdpBound());
        socket.bind(port);
        ASSERT_TRUE(socket.isUdpBound());
        ASSERT_NE(port, socket.getListeningPort());
    }

    /*
     * Test UDPSocket.sendTo() to ensure that the listening socket's UDPSocket.ready() call returns *true*.
     */
    TEST_F(UDPSocketTest, UDPSendAndReady)
    {
        ASSERT_TRUE(socket.bind(0));

        UDPSocket client;

        ASSERT_FALSE(socket.ready());
        const std::string testString = "test";
        ASSERT_TRUE(client.sendTo(LOCALHOST, socket.getListeningPort(), testString).first);
        ASSERT_TRUE(socket.ready());

        client.close();
    }

    /*
     * Call UDPSocket.receiveFrom() to make sure the correct amount of data is read.
     */
    TEST_F(UDPSocketTest, UDPReceiveFrom)
    {
        ASSERT_TRUE(socket.bind(0));

        UDPSocket client;
        const std::string testString = "test";
        ASSERT_TRUE(client.sendTo(LOCALHOST, socket.getListeningPort(), testString).first);

        ASSERT_TRUE(socket.ready());
        std::pair<std::optional<std::string>, kt::SocketAddress> recieved = socket.receiveFrom(testString.size());
        ASSERT_FALSE(socket.ready());
        ASSERT_NE(std::nullopt, recieved.first);
        ASSERT_EQ(testString, recieved.first.value());

        client.close();
    }

    /**
     * Ensure that receiveAmount reads the specified amount even when more is available in the buffer.
     * Also confirm that the remaining data is lost if not read.
     */
    TEST_F(UDPSocketTest, UDPReceiveAmount_NotEnoughRead)
    {
        ASSERT_TRUE(socket.bind(0));

        UDPSocket client;
        const std::string testString = "test";
        ASSERT_FALSE(socket.ready());
        ASSERT_TRUE(client.sendTo(LOCALHOST, socket.getListeningPort(), testString).first);

        ASSERT_TRUE(socket.ready());
        std::pair<std::optional<std::string>, kt::SocketAddress> recieved = socket.receiveFrom(testString.size() - 1);
        ASSERT_FALSE(socket.ready());
        ASSERT_NE(std::nullopt, recieved.first);
        ASSERT_EQ(testString.substr(0, testString.size() - 1), recieved.first.value());

        client.close();
    }

    /*
     * Ensure that the receiveAmount reads the correct amount when less data is provided than expected.
     */
    TEST_F(UDPSocketTest, UDPReceiveAmount_TooMuchRead)
    {
        ASSERT_TRUE(socket.bind(0));

        UDPSocket client;
        const std::string testString = "test";
        ASSERT_TRUE(client.sendTo(LOCALHOST, socket.getListeningPort(), testString).first);

        ASSERT_TRUE(socket.ready());
        std::pair<std::optional<std::string>, kt::SocketAddress> recieved = socket.receiveFrom(testString.size() + 1);
        ASSERT_FALSE(socket.ready());
        ASSERT_NE(std::nullopt, recieved.first);
        ASSERT_EQ(testString, recieved.first.value());

        client.close();
    }

    // TODO: large payload tests
}
