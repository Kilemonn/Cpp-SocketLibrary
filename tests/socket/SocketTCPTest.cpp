#include <string>
#include <optional>

#include <gtest/gtest.h>

#include "../../src/socket/Socket.h"
#include "../../src/socketexceptions/BindingException.hpp"

const std::string LOCALHOST = "127.0.0.1";
const int PORT_NUMBER = 12345;

namespace kt
{
    class SocketTCPTest : public ::testing::Test
    {
    protected:
        Socket socket;

    protected:
        SocketTCPTest() : socket(LOCALHOST, PORT_NUMBER, SocketType::Wifi, SocketProtocol::UDP) { }
        void TearDown() override
        {
            this->socket.close();
        }
    };
}
