#pragma once

#ifdef _WIN32

#include <winsock2.h>

#elif __linux__

#include <arpa/inet.h>

#endif

namespace kt
{
    enum class InternetProtocolVersion
    {
        IPV4 = AF_INET, // 2
        IPV6 = AF_INET6 // 23 / 10?
    };
}
