
#include <gtest/gtest.h>

#include "../../src/socket/UDPSocket.h"
#include "../../src/serversocket/ServerSocket.h"

namespace kt
{
    /**
     * Ensure that we can bind to the same port using TCP and UDP sockets.
     * 
     * In this case, when UDP binds first.
     */
    TEST(ScenarioTest, UDPThenTCPBindSamePort)
    {
        kt::UDPSocket socket;
        std::pair<bool, kt::SocketAddress> bindResult = socket.bind();
        ASSERT_TRUE(bindResult.first);
        ASSERT_NE(std::nullopt, socket.getListeningPort());

        kt::ServerSocket server(SocketType::Wifi, std::nullopt, socket.getListeningPort().value());

        ASSERT_EQ(server.getPort(), socket.getListeningPort().value());

        server.close();
        socket.close();
    }

    /**
     * Ensure that we can bind to the same port using TCP and UDP sockets.
     * 
     * In this case, when TCP binds first.
     */
    TEST(ScenarioTest, TCPThenUDPBindSamePort)
    {
        kt::ServerSocket server(SocketType::Wifi);

        kt::UDPSocket socket;
        std::pair<bool, kt::SocketAddress> bindResult = socket.bind(std::nullopt, server.getPort());
        ASSERT_TRUE(bindResult.first);

        ASSERT_EQ(server.getPort(), socket.getListeningPort().value());

        server.close();
        socket.close();
    }

    /**
     * Ensure that we can bind two UDP sockets to the same address by setting SO_REUSEADDR in the pre-bind handler.
     */
    TEST(ScenarioTest, TwoUDPSocketsBindingToSamePort)
    {
        std::function setReuseAddrOption = [](SOCKET& s) {
            const int enable = 1;
            const int enableOption = 1;
            ASSERT_EQ(0, setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&enableOption, sizeof(enableOption)));
        };

        kt::UDPSocket socket;
        socket.setPreBindSocketOperation(setReuseAddrOption);
        std::pair<bool, kt::SocketAddress> bindResult = socket.bind();
        ASSERT_TRUE(bindResult.first);

        kt::UDPSocket socket2;
        socket2.setPreBindSocketOperation(setReuseAddrOption);
        bindResult = socket2.bind(std::nullopt, socket.getListeningPort().value());
        ASSERT_TRUE(bindResult.first);

        ASSERT_EQ(socket.getListeningPort().value(), socket2.getListeningPort().value());

        kt::UDPSocket sendSocket;
        const std::string data = "TwoUDPSocketsBindingToSamePort";
        std::pair<std::pair<bool, int>, kt::SocketAddress> sendResult = sendSocket.sendTo("localhost", socket.getListeningPort().value(), data);
        ASSERT_TRUE(sendResult.first.first);

        // Make sure only one of the sockets is ready to read, not both
        ASSERT_FALSE(socket.ready() && socket2.ready());
        ASSERT_TRUE(socket2.ready() || socket.ready());

        socket.close();
        socket2.close();

        sendSocket.close();
    }
}
