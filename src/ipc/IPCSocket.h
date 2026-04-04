#pragma once

#include "../socket/ConnectionOrientedSocket.h"

#include <string>
#include <optional>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
#endif

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <afunix.h>

#elif __linux__

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/un.h>

// Typedef to match the windows typedef since they are different underlying types
typedef int SOCKET;

#endif

namespace kt
{
    class IPCSocket : public ConnectionOrientedSocket
    {
        private:
            SOCKET socket;
            std::string socketPath;

            void constructSocket();
        public:
            IPCSocket() = delete;
            IPCSocket(const std::string&);
            IPCSocket(const SOCKET&, const std::string&);

            IPCSocket(const IPCSocket&);
			IPCSocket& operator=(const IPCSocket&);

            SOCKET getSocket() const;
            std::string getSocketPath() const;

            void close() override;

            static void closePath(const std::string&);
    };
} // namespace kt
