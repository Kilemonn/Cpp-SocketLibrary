
#include <gtest/gtest.h>

#include <string>
#include "../../src/socket/Socket.h"

namespace kt
{
    class SocketTest : public ::testing::Test
    {
    public:
        static int PORT_NUMBER;
        static std::string LOCALHOST;

    protected:
         void SetUp() override
         {

         }
        // void TearDown() override {}
    };

    std::string SocketTest::LOCALHOST = "127.0.0.1";
    int SocketTest::PORT_NUMBER = 12345;

    TEST_F(SocketTest, UDPConstructors)
    {
        Socket socket(SocketTest::LOCALHOST, SocketTest::PORT_NUMBER, SocketType::Wifi, SocketProtocol::UDP);
        EXPECT_EQ(socket.getType(), SocketType::Wifi);
        EXPECT_EQ(socket.getProtocol(), SocketProtocol::UDP);
        EXPECT_FALSE(socket.connected());
        EXPECT_FALSE(socket.ready());
        EXPECT_FALSE(socket.isBound());
    }
}

//int main(int argc, char **argv) {
//    ::testing::InitGoogleTest(&argc, argv);
//    return RUN_ALL_TESTS();
//}
