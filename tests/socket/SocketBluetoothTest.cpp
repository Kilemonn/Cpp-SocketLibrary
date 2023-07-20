
#include <string>
#include <optional>

#include <gtest/gtest.h>

#include "../../src/socket/Socket.h"
#include "../../src/serversocket/ServerSocket.h"

namespace kt
{
    class DISABLED_SocketBluetoothTest : public ::testing::Test
    {
    protected:
        ServerSocket serverSocket;
        Socket socket;

    protected:
        DISABLED_SocketBluetoothTest() : serverSocket(SocketType::Bluetooth), socket(Socket::getLocalMACAddress().value(), serverSocket.getPort(), SocketType::Bluetooth) { }
        void TearDown() override
        {
            socket.close();
            serverSocket.close();
        }
    };

    TEST_F(DISABLED_SocketBluetoothTest, BluetoothGetLocalMacAddress)
    {
        GTEST_SKIP();

	    ASSERT_NE(std::nullopt, Socket::getLocalMACAddress());
    }

    TEST_F(DISABLED_SocketBluetoothTest, BluetoothScanDevices)
    {
        GTEST_SKIP();

        std::vector<std::pair<std::string, std::string> > devices = Socket::scanDevices();
        for (const std::pair<std::string, std::string>& p : devices)
        {
            std::cout << p.first << " - " << p.second << std::endl;
        }
    }

    TEST_F(DISABLED_SocketBluetoothTest, BluetoothSend)
    {
        GTEST_SKIP();

        Socket server = serverSocket.accept();
        const std::string toSend = "TestBluetooth";

        ASSERT_TRUE(socket.send(toSend));

        std::string response = server.receiveAmount(toSend.size());
        ASSERT_EQ(response, toSend);

        server.close();
    }
}