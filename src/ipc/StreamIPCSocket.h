#pragma once

#include "../socket/ConnectionOrientedSocket.h"

#include <string>
#include <functional>

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

#else

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/un.h>

// Typedef to match the windows typedef since they are different underlying types
typedef int SOCKET;

#endif

namespace kt
{
    class StreamIPCSocket : public ConnectionOrientedSocket
    {
        private:
            SOCKET socket;
            std::string socketPath;

            void constructSocket(const std::optional<std::function<void(SOCKET&)>>& = std::nullopt);
        public:
            StreamIPCSocket() = delete;
            StreamIPCSocket(const std::string&, const std::optional<std::function<void(SOCKET&)>>& = std::nullopt);
            StreamIPCSocket(const SOCKET&, const std::string&);

            StreamIPCSocket(const StreamIPCSocket&);
			StreamIPCSocket& operator=(const StreamIPCSocket&);

            SOCKET getSocket() const override;
            std::string getSocketPath() const;

            void close() override;
    };
} // namespace kt
