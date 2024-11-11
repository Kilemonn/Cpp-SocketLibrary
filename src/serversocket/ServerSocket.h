#pragma once

#include <optional>

#include "../address/SocketAddress.h"

#include "../socket/BluetoothSocket.h"
#include "../socket/TCPSocket.h"

#include "../enums/SocketType.h"
#include "../enums/InternetProtocolVersion.h"

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

// Typedef to match the windows typedef since they are different underlying types
typedef int SOCKET;

#endif

namespace kt
{
	class ServerSocket
	{
		protected:
			unsigned short port = 0;
			kt::SocketType type = kt::SocketType::None;
			kt::InternetProtocolVersion protocolVersion = kt::InternetProtocolVersion::Any;
			kt::SocketAddress serverAddress = {};
			SOCKET socketDescriptor = getInvalidSocketValue();

			void setDiscoverable();
			void constructSocket(const std::optional<std::string>&, const unsigned int&);
			void constructBluetoothSocket(const unsigned int&);
			void constructWifiSocket(const std::optional<std::string>& localHostname, const unsigned int&);
			void initialisePortNumber();

		public:
			ServerSocket(const kt::SocketType, const std::optional<std::string>& = std::nullopt, const unsigned short& = 0, const unsigned int& = 20, const kt::InternetProtocolVersion = kt::InternetProtocolVersion::Any);
			ServerSocket(const kt::ServerSocket&);
			kt::ServerSocket& operator=(const kt::ServerSocket&);

			kt::TCPSocket acceptTCPConnection(const long& = 0) const;
			kt::BluetoothSocket acceptBluetoothConnection(const long& = 0);

			kt::SocketType getType() const;
			kt::InternetProtocolVersion getInternetProtocolVersion() const;
			unsigned short getPort() const;
			SOCKET getSocket() const;
			kt::SocketAddress getSocketAddress() const;

			void close();
	};

} // End namespace kt
