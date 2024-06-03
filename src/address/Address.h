
#pragma once

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define _WIN32_WINNT 0x501

#include <WinSock2.h>
#include <ws2bth.h>
#include <ws2tcpip.h>

#elif __linux__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

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
}
