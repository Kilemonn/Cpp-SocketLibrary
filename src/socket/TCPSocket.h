#pragma once

#include <iostream>
#include <vector>
#include <utility>
#include <optional>

#include "../enums/SocketType.h"
#include "../enums/InternetProtocolVersion.h"
#include "../address/SocketAddress.h"
#include "../socketexceptions/SocketError.h"

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
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>

// Typedef to match the windows typedef since they are different underlying types
typedef int SOCKET;

#endif

namespace kt
{	class TCPSocket
	{
		protected:
			SOCKET socketDescriptor = getInvalidSocketValue();
			std::string hostname;
			unsigned short port;
			kt::InternetProtocolVersion protocolVersion = kt::InternetProtocolVersion::Any;
			kt::SocketAddress serverAddress = {}; // The remote address that we will be connected to

			void constructWifiSocket();

		public:
			TCPSocket() = default;
			TCPSocket(const std::string&, const unsigned short&);
			TCPSocket(const SOCKET&, const std::string&, const unsigned short&, const kt::InternetProtocolVersion, const kt::SocketAddress&);

			TCPSocket(const kt::TCPSocket&);
			kt::TCPSocket& operator=(const kt::TCPSocket&);
			
			void close() const;
			
			int pollSocket(SOCKET socket, const long& = 100) const;
			bool ready(const unsigned long = 100) const;
			bool connected(const unsigned long = 100) const;
			bool send(const char*, const int&, const int& = 0) const;
			bool send(const std::string&, const int& = 0) const;

			SOCKET getSocket() const;
            std::string getHostname() const;
			unsigned short getPort() const;
			kt::InternetProtocolVersion getInternetProtocolVersion() const;
			kt::SocketAddress getSocketAddress() const;

			std::optional<char> get(const int& = 0) const;
			std::string receiveAmount(const unsigned int, const int& = 0) const;
			int receiveAmount(char*, const unsigned int, const int& = 0) const;
			std::string receiveToDelimiter(const char&, const int& = 0);
			std::string receiveAll(const unsigned long = 100, const int& = 0);
	};

} // End namespace kt 
