#include <string>

#include <gtest/gtest.h>

#include "../../src/ipc/DatagramIPCSocket.h"
#include "../../src/socketexceptions/BindingException.hpp"

const std::string SOCKET_PATH = "/tmp/DatagramIPCSocketTest.sock";

namespace kt
{
#ifdef _WIN32

TEST(DatagramIPCSocketTest, WindowsConstructorThrows)
	{
		ASSERT_THROW(DatagramIPCSocket socket, SocketException);
	}

#else
    class DatagramIPCSocketTest : public ::testing::Test
    {
    protected:
        DatagramIPCSocket socket;

    protected:
        DatagramIPCSocketTest() : socket() { }
        void TearDown() override
        {
            this->socket.close();
        }
    };

    /*
     * Test the kt::DatagramIPCSocket constructors and exception handling for DatagramIPC. This covers the following scenarios:
     * - Constructing a socket and ensuring its default values are set correctly
     */
    TEST_F(DatagramIPCSocketTest, DatagramIPCConstructors)
    {
        ASSERT_FALSE(socket.ready());
        ASSERT_FALSE(socket.isBound());
        ASSERT_EQ(std::nullopt, socket.getSocketPath());
        ASSERT_TRUE(kt::isInvalidSocket(socket.getListeningSocket()));
    }

    TEST_F(DatagramIPCSocketTest, DatagramIPCCopyConstructors)
    {
        ASSERT_EQ(0, socket.bind(true, SOCKET_PATH).first);
        DatagramIPCSocket copiedSocket(socket);

        ASSERT_EQ(socket.getListeningSocket(), copiedSocket.getListeningSocket());
        ASSERT_EQ(socket.isBound(), copiedSocket.isBound());
        ASSERT_EQ(socket.getSocketPath(), copiedSocket.getSocketPath());

        copiedSocket.close();
    }

    TEST_F(DatagramIPCSocketTest, DatagramIPCBindEmptyPath)
    {
        ASSERT_THROW(socket.bind(), BindingException);
    }

    /*
     * Ensure that multiple calls to DatagramIPCSocket.bind() fails if another socket is already bound to that port.
     */
    TEST_F(DatagramIPCSocketTest, DatagramIPCBindAndBound_MultipleCalls)
    {
        ASSERT_FALSE(socket.isBound());
        ASSERT_EQ(0, socket.bind(SOCKET_PATH).first);
        ASSERT_TRUE(socket.isBound());

        kt::DatagramIPCSocket newServer;
        ASSERT_FALSE(newServer.isBound());
        ASSERT_EQ(-1, newServer.bind(SOCKET_PATH).first);
    }

    /*
     * Ensure that multiple calls to DatagramIPCSocket.bind() fails if another socket is already bound to that port.
     */
    TEST_F(DatagramIPCSocketTest, DatagramIPCBindAndBound_Override)
    {
        ASSERT_FALSE(socket.isBound());
        ASSERT_EQ(0, socket.bind(SOCKET_PATH).first);
        ASSERT_TRUE(socket.isBound());

        kt::DatagramIPCSocket newServer;
        ASSERT_FALSE(newServer.isBound());
        ASSERT_EQ(0, newServer.bind(true, SOCKET_PATH).first);
        ASSERT_TRUE(newServer.isBound());

        newServer.close();
    }

    /*
     * Test DatagramIPCSocket.sendTo() to ensure that it can send correctly to the listening socket.
     */
    TEST_F(DatagramIPCSocketTest, DatagramIPCSendTo)
    {
        ASSERT_EQ(0, socket.bind(SOCKET_PATH).first);

        DatagramIPCSocket client;

        ASSERT_FALSE(socket.ready());
        const std::string testString = "test";
        ASSERT_NE(std::nullopt, socket.getSocketPath());
        ASSERT_EQ(client.sendTo(socket.getSocketPath().value(), testString), testString.size());

        while(!socket.ready()) {}
        ASSERT_TRUE(socket.ready());
    }

    /**
     * Ensure that sending with an empty hostname failed to resolve the address and fail.
     */
    TEST_F(DatagramIPCSocketTest, TestEmptyHostname)
    {
        ASSERT_EQ(0, socket.bind(SOCKET_PATH).first);
        
        DatagramIPCSocket client;
        ASSERT_FALSE(socket.ready());
        const std::string message = "test";

        ASSERT_EQ(-1, client.sendTo("", message, 0));
        ASSERT_FALSE(socket.ready());        
    }

    /*
     * Call DatagramIPCSocket.receiveFrom() to make sure the correct amount of data is read.
     */
    TEST_F(DatagramIPCSocketTest, DatagramIPCReceiveFrom)
    {
        ASSERT_EQ(0, socket.bind(SOCKET_PATH).first);
        ASSERT_FALSE(socket.ready());

        DatagramIPCSocket client;
        const std::string testString = "test";
        ASSERT_EQ(client.sendTo(SOCKET_PATH, testString), testString.size());

        while(!socket.ready()) {}
        ASSERT_TRUE(socket.ready());
        std::pair<std::optional<std::string>, std::pair<int, std::string>> recieved = socket.receiveFrom(testString.size());
        ASSERT_FALSE(socket.ready());
        ASSERT_NE(std::nullopt, recieved.first);
        ASSERT_EQ(testString.size(), recieved.second.first);
        ASSERT_EQ(testString, recieved.first.value());
        ASSERT_EQ(SOCKET_PATH, recieved.second.second);
    }

    /**
     * Ensure that receiveAmount reads the specified amount even when more is available in the buffer.
     * Also confirm that the remaining data is lost if not read.
     */
    TEST_F(DatagramIPCSocketTest, DatagramIPCReceiveAmount_NotEnoughRead)
    {
        ASSERT_EQ(0, socket.bind(SOCKET_PATH).first);
        ASSERT_FALSE(socket.ready());

        DatagramIPCSocket client;
        const std::string testString = "test";
        ASSERT_EQ(client.sendTo(SOCKET_PATH, testString), testString.size());

        while(!socket.ready()) {}
        ASSERT_TRUE(socket.ready());
        std::pair<std::optional<std::string>, std::pair<int, std::string>> recieved = socket.receiveFrom(testString.size() - 1);
        ASSERT_FALSE(socket.ready());
        ASSERT_NE(std::nullopt, recieved.first);
        ASSERT_EQ(testString.substr(0, testString.size() - 1), recieved.first.value());
    }

    /*
     * Ensure that the receiveAmount reads the correct amount when less data is provided than expected.
     */
    TEST_F(DatagramIPCSocketTest, DatagramIPCReceiveAmount_TooMuchRead)
    {
        ASSERT_EQ(0, socket.bind(SOCKET_PATH).first);
        ASSERT_FALSE(socket.ready());

        DatagramIPCSocket client;
        const std::string testString = "test";
        ASSERT_EQ(client.sendTo(SOCKET_PATH, testString), testString.size());

        while(!socket.ready()) {}
        ASSERT_TRUE(socket.ready());
        std::pair<std::optional<std::string>, std::pair<int, std::string>> recieved = socket.receiveFrom(testString.size() + 1);
        ASSERT_FALSE(socket.ready());
        ASSERT_NE(std::nullopt, recieved.first);
        ASSERT_EQ(testString, recieved.first.value());
    }

    // Use the returned .bind() address to use as the socket address that is used to send a message
    TEST_F(DatagramIPCSocketTest, SendToBoundAddress)
    {
        std::pair<int, std::string> bindResult = socket.bind(SOCKET_PATH);
        ASSERT_EQ(0, bindResult.first);

        DatagramIPCSocket client;
        ASSERT_FALSE(socket.ready());
        const std::string message = "SendToBoundAddress";
        ASSERT_EQ(client.sendTo(bindResult.second, message), message.size());

        while(!socket.ready()) {}
        ASSERT_TRUE(socket.ready());
    }
#endif
}
