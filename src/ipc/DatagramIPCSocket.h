#pragma once

#include "../socket/ConnectionLessSocket.h"
#include "IPCSocket.h"
#include "../socketexceptions/SocketError.h"

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
    class DatagramIPCSocket : ConnectionLessSocket<std::string>, public IPCSocket
    {
        private:
            bool bound = false;
            std::optional<std::string> socketPath = std::nullopt;
            SOCKET receiveSocket = getInvalidSocketValue();
            std::optional<std::function<void(SOCKET&)>> preSendSocketOperation = std::nullopt;

        public:
            DatagramIPCSocket() = default;

            using ConnectionLessSocket::bind;
            std::pair<int, std::string> bind(const std::optional<std::string>& = std::nullopt, const std::optional<std::function<void(SOCKET&)>>& = std::nullopt) override;
            bool isBound() const override;

            void setPreSendSocketOperation(std::function<void(SOCKET&)>);

		    void close() override;
    };
}
