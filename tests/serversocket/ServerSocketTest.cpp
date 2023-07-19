
#include <gtest/gtest.h>

#include "../../src/serversocket/ServerSocket.h"
#include "../../src/socketexceptions/BindingException.hpp"

namespace kt
{
    class ServerSocketTest: public ::testing::Test
    {
    public:
        static int PORT_NUMBER;

    protected:
        void SetUp() override
        {

        }

        // void TearDown() override {}
    };

    int ServerSocketTest::PORT_NUMBER = 87682;

    TEST_F(ServerSocketTest, TestConstructors)
    {
        ServerSocket server(SocketType::Wifi, ServerSocketTest::PORT_NUMBER);

        // Ensure a binding exception is thrown if another process, (in this case another server) is using the port
        EXPECT_THROW({
            ServerSocket server2(SocketType::Wifi, server.getPort());
            }, BindingException);
    }
}

