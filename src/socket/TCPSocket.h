#pragma once

#include <iostream>
#include <vector>
#include <utility>
#include <optional>

#include "../enums/InternetProtocolVersion.h"
#include "../address/SocketAddress.h"
#include "../socketexceptions/SocketError.h"
#include "ConnectionOrientedSocket.h"

#include "Socket.h"

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
#include <unistd.h>
#include <netdb.h>

// Typedef to match the windows typedef since they are different underlying types
typedef int SOCKET;

#endif

namespace kt
{	class TCPSocket : public ConnectionOrientedSocket
	{
		protected:
			SOCKET socketDescriptor = getInvalidSocketValue();
			std::string hostname;
			unsigned short port;
			kt::InternetProtocolVersion protocolVersion = kt::InternetProtocolVersion::Any;
			kt::SocketAddress serverAddress = {}; // The remote address that we will be connected to

			void constructSocket();

		public:
			TCPSocket() = delete;
			TCPSocket(const std::string&, const unsigned short&, const kt::InternetProtocolVersion = kt::InternetProtocolVersion::Any);
			TCPSocket(const SOCKET&, const std::string&, const unsigned short&, const kt::InternetProtocolVersion, const kt::SocketAddress&);
			TCPSocket(const kt::SocketAddress);

			TCPSocket(const kt::TCPSocket&);
			kt::TCPSocket& operator=(const kt::TCPSocket&);

			SOCKET getSocket() const override;
            std::string getHostname() const;
			unsigned short getPort() const;
			kt::InternetProtocolVersion getInternetProtocolVersion() const;
			kt::SocketAddress getSocketAddress() const;

			using ConnectionOrientedSocket::receiveAmount;

			void close() override;
	};

} // End namespace kt 
