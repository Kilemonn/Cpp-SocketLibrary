#include <iostream>
#include <fstream>
#include <thread>
#include <exception>
#include <cassert>
#include <stdexcept>
#include <functional>

#include "../Socket/Socket.h"
#include "../ServerSocket/ServerSocket.h"
#include "../SocketExceptions/SocketException.hpp"
#include "../SocketExceptions/BindingException.hpp"
#include "../Enums/SocketProtocol.cpp"
#include "../Enums/SocketType.cpp"

#include "TestUtil.hpp"

const int PORT_NUMBER = 12345;
const std::string LOCALHOST = "127.0.0.1";

void testWifiConstructors()
{
    preFunctionTest(__func__);


}

int main()
{
    testFunction(testWifiConstructors);
}
