
#include <gtest/gtest.h>

#include "../../src/serversocket/ServerSocket.h"
#include "../../src/socketexceptions/BindingException.hpp"
#include "../TestUtil.hpp"

namespace kt
{
    class ServerSocketTest: public ::testing::Test
    {
    protected:
        void SetUp() override
        {

        }

        // void TearDown() override {}
    };

    TEST_F(ServerSocketTest, TestConstructors)
    {
        const unsigned int PORT = 87682;
        ServerSocket server(SocketType::Wifi, PORT);

        // Ensure a binding exception is thrown if another process, (in this case another server) is using the port
        assert(throwsException<BindingException>([&server]
        {
            ServerSocket server2(SocketType::Wifi, server.getPort());
        }));
    }
}

