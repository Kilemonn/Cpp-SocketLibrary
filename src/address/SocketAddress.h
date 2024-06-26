
#pragma once

#include "../enums/InternetProtocolVersion.h"

#include <string>
#include <optional>
#include <vector>
#include <cstring>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
#endif

#include <WinSock2.h>
#include <ws2bth.h>
#include <ws2tcpip.h>

#elif __linux__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>

// Typedef to match the windows typedef since they are different underlying types
typedef int SOCKET;

#endif

namespace kt 
{
    /**
     * A union object that represents all different sockaddr representations for both ipv4 and ipv6.
    */
    typedef union
    {
        sockaddr address;
        sockaddr_in ipv4;
        sockaddr_in6 ipv6;
    } SocketAddress;

    kt::InternetProtocolVersion getInternetProtocolVersion(const kt::SocketAddress&);

    unsigned int getPortNumber(const kt::SocketAddress&);

    std::optional<std::string> getAddress(const kt::SocketAddress&);

    std::pair<std::optional<kt::SocketAddress>, int> socketToAddress(const SOCKET&);

    std::pair<std::vector<kt::SocketAddress>, int> resolveToAddresses(const std::optional<std::string>&, const unsigned int&, addrinfo&);

#ifdef _WIN32
	int getAddressLength(const kt::SocketAddress&);
#else
	socklen_t getAddressLength(const kt::SocketAddress&);
#endif
}
