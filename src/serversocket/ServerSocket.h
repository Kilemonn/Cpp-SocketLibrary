#pragma once

#include <optional>

#include "../address/SocketAddress.h"

#include "../socket/Socket.h"

#include "../enums/SocketProtocol.h"
#include "../enums/SocketType.h"
#include "../enums/InternetProtocolVersion.h"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define _WIN32_WINNT 0x0600

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
			unsigned int port;
			kt::SocketType type = kt::SocketType::None;
			kt::InternetProtocolVersion protocolVersion = kt::InternetProtocolVersion::Any;
			kt::SocketAddress serverAddress = {};
			SOCKET socketDescriptor = getInvalidSocketValue();

			void setDiscoverable();
			void constructSocket(const unsigned int&);
			void constructBluetoothSocket(const unsigned int&);
			void constructWifiSocket(const unsigned int&);
			void initialisePortNumber();
			size_t initialiseServerAddress();

			kt::Socket acceptWifiConnection(const long& = 0);
			kt::Socket acceptBluetoothConnection(const long& = 0);

		public:
			ServerSocket() = default;
			ServerSocket(const kt::SocketType, const unsigned int& = 0, const unsigned int& = 20, const kt::InternetProtocolVersion = kt::InternetProtocolVersion::Any);
			ServerSocket(const kt::ServerSocket&);
			kt::ServerSocket& operator=(const kt::ServerSocket&);

			kt::SocketType getType() const;
			kt::InternetProtocolVersion getInternetProtocolVersion() const;
			unsigned int getPort() const;

			kt::Socket accept(const long& = 0);
			void close();
	};

} // End namespace kt
