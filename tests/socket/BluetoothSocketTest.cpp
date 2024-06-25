
#include <string>
#include <optional>

#include <gtest/gtest.h>

#include "../../src/socket/BluetoothSocket.h"
#include "../../src/serversocket/ServerSocket.h"

namespace kt
{
    class DISABLED_BluetoothSocketTest : public ::testing::Test
    {
    protected:
        ServerSocket serverSocket;
        BluetoothSocket socket;

    protected:
        DISABLED_BluetoothSocketTest() : serverSocket(SocketType::Bluetooth), socket(BluetoothSocket::getLocalMACAddress().value(), serverSocket.getPort()) { }
        // void TearDown() override
        // {
        //     socket.close();
        //     serverSocket.close();
        // }
    };

    TEST_F(DISABLED_BluetoothSocketTest, BluetoothGetLocalMacAddress)
    {
        GTEST_SKIP();

	    ASSERT_NE(std::nullopt, BluetoothSocket::getLocalMACAddress());
    }

    TEST_F(DISABLED_BluetoothSocketTest, BluetoothScanDevices)
    {
        GTEST_SKIP();

        std::vector<std::pair<std::string, std::string> > devices = BluetoothSocket::scanDevices();
        for (const std::pair<std::string, std::string>& p : devices)
        {
            std::cout << p.first << " - " << p.second << std::endl;
        }
    }

    TEST_F(DISABLED_BluetoothSocketTest, BluetoothSend)
    {
        GTEST_SKIP();

        BluetoothSocket server = serverSocket.acceptBluetoothConnection();
        const std::string toSend = "TestBluetooth";

        ASSERT_TRUE(socket.send(toSend));

        std::string response = server.receiveAmount(toSend.size());
        ASSERT_EQ(response, toSend);

        server.close();
    }
}