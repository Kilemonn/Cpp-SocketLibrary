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
            DatagramIPCSocket();

            using ConnectionLessSocket::bind;
            std::pair<int, std::string> bind(const std::optional<std::string>& = std::nullopt, const std::optional<std::function<void(SOCKET&)>>& = std::nullopt) override;
            std::pair<int, std::string> bind(const bool&, const std::optional<std::string>& = std::nullopt, const std::optional<std::function<void(SOCKET&)>>& = std::nullopt);
            bool isBound() const override;

            SOCKET getListeningSocket() const;
            std::optional<std::string> getSocketPath() const;

            void setPreSendSocketOperation(std::function<void(SOCKET&)>);

            bool ready(const unsigned long = 100) const override;

            int sendTo(const std::string&, const std::string&, const int& = 0) override;
            int sendTo(const std::string&, const char*, const int&, const int& = 0) override;
            
            std::pair<std::optional<std::string>, std::pair<int, std::string>> receiveFrom(const int&, const int& = 0) override;
            std::pair<int, std::string> receiveFrom(char*, const int&, const int& = 0) const override;

		    void close() override;
    };
}
