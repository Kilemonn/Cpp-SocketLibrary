
#pragma once

#include "../enums/InternetProtocolVersion.h"

#include <string>
#include <optional>
#include <vector>
#include <expected>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
#endif

#include <WinSock2.h>
#include <ws2tcpip.h>

#else

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
    union SocketAddress
    {
        sockaddr address;
        sockaddr_in ipv4;
        sockaddr_in6 ipv6;
        
        kt::InternetProtocolVersion getInternetProtocolVersion() const;
        unsigned short getPortNumber() const;
        std::optional<std::string> getAddress() const;
        
#ifdef _WIN32
        int getAddressLength() const;
#else
        socklen_t getAddressLength() const;
#endif
    };
    
    std::expected<kt::SocketAddress, int> socketToAddress(const SOCKET&);

    std::expected<std::vector<kt::SocketAddress>, int> resolveToAddresses(const std::string&, const unsigned short&, addrinfo&);

    addrinfo createUdpHints(const kt::InternetProtocolVersion = kt::InternetProtocolVersion::Any, const int = 0);

    addrinfo createTcpHints(const kt::InternetProtocolVersion = kt::InternetProtocolVersion::Any, const int = 0);

    std::optional<std::string> getEmptyAddress(const kt::InternetProtocolVersion);

    std::string getLocalAddress(const kt::InternetProtocolVersion);
}
