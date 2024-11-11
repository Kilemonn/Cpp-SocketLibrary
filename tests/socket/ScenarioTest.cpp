
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
    }
}
