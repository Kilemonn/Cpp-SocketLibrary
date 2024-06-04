
#include <string>
#include <optional>

#include <gtest/gtest.h>

#include "../../src/socket/Socket.h"
#include "../../src/socketexceptions/BindingException.hpp"

const std::string LOCALHOST = "localhost";
const int PORT_NUMBER = 12345;

namespace kt
{
    class SocketUDPTest : public ::testing::Test
    {
    protected:
        Socket socket;

    protected:
        SocketUDPTest() : socket(LOCALHOST, PORT_NUMBER, SocketType::Wifi, SocketProtocol::UDP) { }
        void TearDown() override
        {
            this->socket.close();
        }
    };

    /*
     * Test the kt::Socket constructors and exception handling for UDP. This covers the following scenarios:
     * - Constructing a socket and ensuring its default values are set correctly
     */
    TEST_F(SocketUDPTest, UDPConstructors)
    {
        ASSERT_EQ(socket.getType(), SocketType::Wifi);
        ASSERT_EQ(socket.getProtocol(), SocketProtocol::UDP);
        ASSERT_FALSE(socket.connected());
        ASSERT_FALSE(socket.ready());
        ASSERT_FALSE(socket.isBound());
        ASSERT_EQ(LOCALHOST, socket.getAddress());
        ASSERT_EQ(std::nullopt, socket.getLastRecievedAddress());
    }

    /*
     * Ensure that we cannot receive any messages when we are not bound.
     */
    TEST_F(SocketUDPTest, UDPBind_NotCalled)
    {
        ASSERT_FALSE(socket.isBound());

        kt::Socket client(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::UDP);

        ASSERT_FALSE(socket.ready());
        client.send("test");
        ASSERT_FALSE(socket.ready());
    }

    /*
     * Ensure that multiple calls to Socket.bind() fails if another socket is already bound to that port.
     */
    TEST_F(SocketUDPTest, UDPBindAndBound_MultipleCalls)
    {
        ASSERT_FALSE(socket.isBound());
        socket.bind();
        ASSERT_TRUE(socket.isBound());

        kt::Socket newServer(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
        ASSERT_FALSE(newServer.isBound());
        EXPECT_THROW(newServer.bind(), BindingException);
    }

    /*
     * Ensure that multiple calls to Socket.bind() fails if another socket is already bound to that port.
     */
    TEST_F(SocketUDPTest, UDPBind_WithoutSpecifiedPort)
    {
        ASSERT_FALSE(socket.isBound());

        unsigned int port = 0;
        kt::Socket newServer(LOCALHOST, port, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
        ASSERT_FALSE(newServer.isBound());
        newServer.bind();
        ASSERT_TRUE(newServer.isBound());
        ASSERT_NE(port, newServer.getPort()); // Make sure we have looked up and resolved the port number upon successful binding

        newServer.close();
    }

    /*
     * Test Socket.send(std::string) to ensure that the server socket receives data.
     */
    TEST_F(SocketUDPTest, UDPSendAndReady)
    {
        socket.bind();

        Socket client(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::UDP);

        ASSERT_FALSE(socket.ready());
        const std::string testString = "test";
        ASSERT_TRUE(client.send(testString));
        ASSERT_TRUE(socket.ready());

        client.close();
    }

    /*
     * Call receiveAmount to make sure the correct amount is read.
     */
    TEST_F(SocketUDPTest, UDPReceiveAmount)
    {
        socket.bind();

        Socket client(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
        const std::string testString = "test";
        ASSERT_TRUE(client.send(testString));

        ASSERT_TRUE(socket.ready());
        std::string recieved = socket.receiveAmount(testString.size());
        ASSERT_FALSE(socket.ready());
        ASSERT_EQ(testString, recieved);

        client.close();
    }

    /*
     * Ensure that receiveAmount reads the specified amount even when more is available in the buffer.
     * Also confirm that the remaining data is lost if not read.
     */
    TEST_F(SocketUDPTest, UDPReceiveAmount_NotEnoughRead)
    {
        socket.bind();

        Socket client(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
        const std::string testString = "test";
        ASSERT_FALSE(socket.ready());
        ASSERT_TRUE(client.send(testString));

        ASSERT_TRUE(socket.ready());
        std::string recieved = socket.receiveAmount(testString.size() - 1);
        ASSERT_FALSE(socket.ready());
        ASSERT_EQ(testString.substr(0, testString.size() - 1), recieved);

        client.close();
    }

    /*
     * Ensure that the receiveAmount read the correct amount when less data is provided than expected.
     */
    TEST_F(SocketUDPTest, UDPReceiveAmount_TooMuchRead)
    {
        socket.bind();

        Socket client(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
        const std::string testString = "test";
        ASSERT_TRUE(client.send(testString));

        ASSERT_TRUE(socket.ready());
        std::string recieved = socket.receiveAmount(testString.size() + 1);
        ASSERT_FALSE(socket.ready());
        ASSERT_EQ(testString, recieved);

        client.close();
    }

    /*
     * Ensure that the last received address is set accordingly after data is read.
     */
    TEST_F(SocketUDPTest, UDPGetAndLastReceivedAddress)
    {
        socket.bind();

        const std::string address = "127.0.0.1";
        Socket client(address, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
        std::string testString = "t";
        ASSERT_TRUE(client.send(testString));

        ASSERT_TRUE(socket.ready());
        std::string received = std::string(1, *socket.get());
        ASSERT_FALSE(socket.ready());
        ASSERT_EQ(testString, received);

        ASSERT_EQ(address, socket.getLastRecievedAddress());

        client.close();
    }

    /*
     * Ensure that the last received address is set accordingly after data is read using IPV6.
     */
    TEST_F(SocketUDPTest, UDPGetAndLastReceivedAddress_IPV6)
    {
        socket.close();

        const std::string address = "::1";
        socket = Socket(address, 0, SocketType::Wifi, SocketProtocol::UDP, InternetProtocolVersion::IPV6);
        socket.bind();

        Socket client(address, socket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::UDP, InternetProtocolVersion::IPV6);
        std::string testString = "t";
        ASSERT_TRUE(client.send(testString));

        ASSERT_TRUE(socket.ready());
        std::string received = std::string(1, *socket.get());
        ASSERT_FALSE(socket.ready());
        ASSERT_EQ(testString, received);

        ASSERT_EQ(address, socket.getLastRecievedAddress());

        client.close();
    }

    /*
     * Throw in ReceiveAll for UDP, since ReceiveAmount should be used instead.
     */
    TEST_F(SocketUDPTest, UDPReceiveAll)
    {
        socket.bind();

        Socket client(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
        const std::string testString = "testString";
        ASSERT_TRUE(client.send(testString));
        
        ASSERT_TRUE(socket.ready());
        ASSERT_THROW({
            std::string response = socket.receiveAll();
            ASSERT_FALSE(socket.ready());
        }, SocketException);

        client.close();
    }

    /*
     * Ensure receive to delimiter receives until the delimiter. 
     */
    TEST_F(SocketUDPTest, UDPReceiveToDelimiter)
    {
        socket.bind();

        Socket client(LOCALHOST, PORT_NUMBER, kt::SocketType::Wifi, kt::SocketProtocol::UDP);

        const std::string testString = "testString";
        const char delimiter = '~';
        ASSERT_TRUE(client.send(testString + delimiter + testString));
        
        ASSERT_TRUE(socket.ready());
        std::string response = socket.receiveToDelimiter(delimiter);
        ASSERT_FALSE(socket.ready()); // Parts of the message past the delimiter will be lost in UDP
        ASSERT_EQ(response, testString);

        client.close();
    }
    
}
