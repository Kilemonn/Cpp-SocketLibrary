
#include <string>
#include <optional>

#include <gtest/gtest.h>

#include "../../src/socket/Socket.h"
#include "../../src/socketexceptions/BindingException.hpp"

const std::string LOCALHOST = "localhost";

namespace kt
{
    class SocketUDPTest : public ::testing::Test
    {
    protected:
        Socket socket;

    protected:
        SocketUDPTest() : socket(LOCALHOST, 0, SocketType::Wifi, SocketProtocol::UDP) { }
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
        ASSERT_FALSE(socket.isUdpBound());
        ASSERT_EQ(LOCALHOST, socket.getHostname());
        ASSERT_EQ(std::nullopt, socket.getLastUDPRecievedAddress());
    }

    /*
     * Ensure that we cannot receive any messages when we are not bound.
     */
    TEST_F(SocketUDPTest, UDPBind_NotCalled)
    {
        ASSERT_FALSE(socket.isUdpBound());

        kt::Socket client(LOCALHOST, socket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::UDP);

        ASSERT_FALSE(socket.ready());
        client.send("test");
        ASSERT_FALSE(socket.ready());
    }

    /*
     * Ensure that multiple calls to Socket.bind() fails if another socket is already bound to that port.
     */
    TEST_F(SocketUDPTest, UDPBindAndBound_MultipleCalls)
    {
        ASSERT_FALSE(socket.isUdpBound());
        ASSERT_TRUE(socket.bind());
        ASSERT_TRUE(socket.isUdpBound());

        kt::Socket newServer(LOCALHOST, socket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::UDP);
        ASSERT_FALSE(newServer.isUdpBound());
        EXPECT_THROW(newServer.bind(), BindingException);
    }

    /*
     * Ensure that multiple calls to Socket.bind() fails if another socket is already bound to that port.
     */
    TEST_F(SocketUDPTest, UDPBind_WithoutSpecifiedPort)
    {
        ASSERT_FALSE(socket.isUdpBound());

        unsigned int port = 0;
        kt::Socket newServer(LOCALHOST, port, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
        ASSERT_FALSE(newServer.isUdpBound());
        newServer.bind();
        ASSERT_TRUE(newServer.isUdpBound());
        ASSERT_NE(port, newServer.getPort()); // Make sure we have looked up and resolved the port number upon successful binding

        newServer.close();
    }

    /*
     * Test Socket.send(std::string) to ensure that the server socket receives data.
     */
    TEST_F(SocketUDPTest, UDPSendAndReady)
    {
        ASSERT_TRUE(socket.bind());

        Socket client(LOCALHOST, socket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::UDP);

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
        ASSERT_TRUE(socket.bind());

        Socket client(LOCALHOST, socket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::UDP);
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
        ASSERT_TRUE(socket.bind());

        Socket client(LOCALHOST, socket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::UDP);
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
        ASSERT_TRUE(socket.bind());

        Socket client(LOCALHOST, socket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::UDP);
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
        ASSERT_TRUE(socket.bind(InternetProtocolVersion::IPV4));

        const std::string address = "127.0.0.1";
        Socket client(address, socket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::UDP);
        std::string testString = "t";
        ASSERT_TRUE(client.send(testString));

        ASSERT_TRUE(socket.ready());
        std::string received = std::string(1, *socket.get());
        ASSERT_FALSE(socket.ready());
        ASSERT_EQ(testString, received);

        ASSERT_EQ(address, socket.getLastUDPRecievedAddress());
        ASSERT_NE(std::nullopt, socket.getLastUDPReceivedAddress());
        ASSERT_EQ(InternetProtocolVersion::IPV4, kt::getInternetProtocolVersion(socket.getLastUDPReceivedAddress().value()));


        client.close();
    }

    /*
     * Ensure that the last received address is set accordingly after data is read using IPV6.
     */
    TEST_F(SocketUDPTest, UDPGetAndLastReceivedAddress_IPV6)
    {
        ASSERT_TRUE(socket.bind(InternetProtocolVersion::IPV6));

        const std::string address = "::1";
        Socket client(address, socket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::UDP);
        std::string testString = "t";
        ASSERT_TRUE(client.send(testString));

        ASSERT_TRUE(socket.ready());
        std::string received = std::string(1, *socket.get());
        ASSERT_FALSE(socket.ready());
        ASSERT_EQ(testString, received);

        ASSERT_EQ(address, socket.getLastUDPRecievedAddress());
        ASSERT_NE(std::nullopt, socket.getLastUDPReceivedAddress());
        ASSERT_EQ(InternetProtocolVersion::IPV6, kt::getInternetProtocolVersion(socket.getLastUDPReceivedAddress().value()));

        client.close();
    }

    /*
     * Throw in ReceiveAll for UDP, since ReceiveAmount should be used instead.
     */
    TEST_F(SocketUDPTest, UDPReceiveAll)
    {
        ASSERT_TRUE(socket.bind());

        Socket client(LOCALHOST, socket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::UDP);
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
        ASSERT_TRUE(socket.bind());

        Socket client(LOCALHOST, socket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::UDP);

        const std::string testString = "testString";
        const char delimiter = '~';
        ASSERT_TRUE(client.send(testString + delimiter + testString));
        
        ASSERT_TRUE(socket.ready());
        std::string response = socket.receiveToDelimiter(delimiter);
        ASSERT_FALSE(socket.ready()); // Parts of the message past the delimiter will be lost in UDP
        ASSERT_EQ(response, testString);

        client.close();
    }

    TEST_F(SocketUDPTest, UDPSendToIPV4ThenToIPV6UsingSetUDPSendAddress)
    {
        ASSERT_TRUE(socket.bind());
        Socket ipv6Socket("::1", 0, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
        ASSERT_TRUE(ipv6Socket.bind(kt::InternetProtocolVersion::IPV6));

        Socket client(LOCALHOST, socket.getPort(), kt::SocketType::Wifi, kt::SocketProtocol::UDP);

        // Confirm we sent the message to the ipv4 udp socket
        std::string testString = "testString";
        ASSERT_TRUE(client.send(testString));

        ASSERT_TRUE(socket.ready());
        std::string response = socket.receiveAmount(testString.size());
        ASSERT_FALSE(socket.ready()); // Parts of the message past the delimiter will be lost in UDP
        ASSERT_EQ(response, testString);

        std::optional<kt::SocketAddress> ipv4Address = client.getUDPSendAddress();
        ASSERT_NE(std::nullopt, ipv4Address);
        
        // Confirm we sent the message to the ipv6 udp socket by resolving it
        client.setUDPSendAddress("::1", ipv6Socket.getPort());
        testString += "2";
        ASSERT_TRUE(client.send(testString));

        ASSERT_TRUE(ipv6Socket.ready());
        response = ipv6Socket.receiveAmount(testString.size());
        ASSERT_FALSE(ipv6Socket.ready());
        ASSERT_EQ(response, testString);

        // Switch back to the ipv4 socket using a different override and confirm we can send to that socket
        client.setUDPSendAddress(ipv4Address.value());
        testString += "45";
        ASSERT_TRUE(client.send(testString));

        ASSERT_TRUE(socket.ready());
        response = socket.receiveAmount(testString.size());
        ASSERT_FALSE(socket.ready());
        ASSERT_EQ(response, testString);

        client.close();
        ipv6Socket.close();
    }

    TEST_F(SocketUDPTest, UDPReturnMessageToSender)
    {
        ASSERT_TRUE(socket.bind(kt::InternetProtocolVersion::IPV4));
        Socket ipv6Socket(LOCALHOST, 0, kt::SocketType::Wifi, kt::SocketProtocol::UDP);
        ASSERT_TRUE(ipv6Socket.bind(kt::InternetProtocolVersion::IPV6));
        ipv6Socket.setUDPSendAddress(LOCALHOST, socket.getPort(), kt::InternetProtocolVersion::IPV4);

        // Send from ipv6 to ipv4 (since ipv6 has solved the ipv4 host in the constructor)
        std::string payload = "my_payload";
        ASSERT_TRUE(ipv6Socket.send(payload));

        ASSERT_TRUE(socket.ready());
        std::string response = socket.receiveAmount(payload.size());
        ASSERT_FALSE(socket.ready());
        ASSERT_EQ(response, payload);

        std::optional<kt::SocketAddress> receivedAddressOpt = socket.getLastUDPReceivedAddress();
        ASSERT_NE(std::nullopt, receivedAddressOpt);
        kt::SocketAddress receivedAddress = receivedAddressOpt.value();
        // Because socket is an ipv4 listening socket, the receive address will also be ipv4
        ASSERT_EQ(kt::InternetProtocolVersion::IPV4, kt::getInternetProtocolVersion(receivedAddress));
        
        // If we want to use the received address in the below call we need to inline edit the address to be an ipv6 address
        // We can do this by adding: "::ffff:" as a prefix to the existing address, but would want to do it directly in the struct
        // socket.setUDPSendAddress(receivedAddress);
        socket.setUDPSendAddress("::1", ipv6Socket.getPort(), kt::InternetProtocolVersion::IPV6);

        ASSERT_TRUE(socket.send(response));

        ASSERT_TRUE(ipv6Socket.ready());
        payload = ipv6Socket.receiveAmount(payload.size());
        ASSERT_FALSE(ipv6Socket.ready());
        ASSERT_EQ(response, payload);

        ipv6Socket.close();
    }

    // TODO: large payload tests
}
