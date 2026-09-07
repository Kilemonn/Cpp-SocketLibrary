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
        ASSERT_TRUE(socket.bind(true, SOCKET_PATH));
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
        ASSERT_TRUE(socket.bind(SOCKET_PATH));
        ASSERT_TRUE(socket.isBound());

        kt::DatagramIPCSocket newServer;
        ASSERT_FALSE(newServer.isBound());
        ASSERT_FALSE(newServer.bind(SOCKET_PATH));
    }

    /*
     * Ensure that multiple calls to DatagramIPCSocket.bind() fails if another socket is already bound to that port.
     */
    TEST_F(DatagramIPCSocketTest, DatagramIPCBindAndBound_Override)
    {
        ASSERT_FALSE(socket.isBound());
        ASSERT_TRUE(socket.bind(SOCKET_PATH));
        ASSERT_TRUE(socket.isBound());

        kt::DatagramIPCSocket newServer;
        ASSERT_FALSE(newServer.isBound());
        ASSERT_TRUE(newServer.bind(true, SOCKET_PATH));
        ASSERT_TRUE(newServer.isBound());

        newServer.close();
    }

    /*
     * Test DatagramIPCSocket.sendTo() to ensure that it can send correctly to the listening socket.
     */
    TEST_F(DatagramIPCSocketTest, DatagramIPCSendTo)
    {
        ASSERT_TRUE(socket.bind(SOCKET_PATH));

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
        ASSERT_TRUE(socket.bind(SOCKET_PATH));
        
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
        ASSERT_TRUE(socket.bind(SOCKET_PATH));
        ASSERT_FALSE(socket.ready());

        DatagramIPCSocket client;
        const std::string testString = "test";
        ASSERT_EQ(client.sendTo(SOCKET_PATH, testString), testString.size());

        while(!socket.ready()) {}
        ASSERT_TRUE(socket.ready());
        std::expected<std::pair<std::string, std::string>, int> recieved = socket.receiveFrom(testString.size());
        ASSERT_FALSE(socket.ready());
        ASSERT_TRUE(recieved);
        ASSERT_EQ(testString.size(), recieved.value().first.size());
        ASSERT_EQ(testString, recieved.value().first);
        ASSERT_EQ(SOCKET_PATH, recieved.value().second);
    }

    /**
     * Ensure that receiveAmount reads the specified amount even when more is available in the buffer.
     * Also confirm that the remaining data is lost if not read.
     */
    TEST_F(DatagramIPCSocketTest, DatagramIPCReceiveAmount_NotEnoughRead)
    {
        ASSERT_TRUE(socket.bind(SOCKET_PATH));
        ASSERT_FALSE(socket.ready());

        DatagramIPCSocket client;
        const std::string testString = "test";
        ASSERT_EQ(client.sendTo(SOCKET_PATH, testString), testString.size());

        while(!socket.ready()) {}
        ASSERT_TRUE(socket.ready());
        std::expected<std::pair<std::string, std::string>, int> recieved = socket.receiveFrom(testString.size() - 1);
        ASSERT_FALSE(socket.ready());
        ASSERT_TRUE(recieved);
        ASSERT_EQ(testString.substr(0, testString.size() - 1), recieved.value().first);
    }

    /*
     * Ensure that the receiveAmount reads the correct amount when less data is provided than expected.
     */
    TEST_F(DatagramIPCSocketTest, DatagramIPCReceiveAmount_TooMuchRead)
    {
        ASSERT_TRUE(socket.bind(SOCKET_PATH));
        ASSERT_FALSE(socket.ready());

        DatagramIPCSocket client;
        const std::string testString = "test";
        ASSERT_EQ(client.sendTo(SOCKET_PATH, testString), testString.size());

        while(!socket.ready()) {}
        ASSERT_TRUE(socket.ready());
        std::expected<std::pair<std::string, std::string>, int> recieved = socket.receiveFrom(testString.size() + 1);
        ASSERT_FALSE(socket.ready());
        ASSERT_TRUE(recieved);
        ASSERT_EQ(testString, recieved.value().first);
    }

    // Use the returned .bind() address to use as the socket address that is used to send a message
    TEST_F(DatagramIPCSocketTest, SendToBoundAddress)
    {
        std::expected<std::string, int> bindResult = socket.bind(SOCKET_PATH);
        ASSERT_TRUE(bindResult);

        DatagramIPCSocket client;
        ASSERT_FALSE(socket.ready());
        const std::string message = "SendToBoundAddress";
        ASSERT_EQ(client.sendTo(bindResult.value(), message), message.size());

        while(!socket.ready()) {}
        ASSERT_TRUE(socket.ready());
    }
#endif
}
