#pragma once

#include <iostream>
#include <vector>
#include <utility>
#include <optional>

#include "../enums/SocketProtocol.h"
#include "../enums/SocketType.h"
#include "../enums/InternetProtocolVersion.h"
#include "../address/SocketAddress.h"
#include "../socketexceptions/SocketError.h"

#include "Socket.h"

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
#include <unistd.h>
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>

// Typedef to match the windows typedef since they are different underlying types
typedef int SOCKET;

#endif

namespace kt
{
	const unsigned int DEFAULT_UDP_BUFFER_SIZE = 10240; // 10 kilobytes

	class UDPSocket
	{
	protected:
		bool bound = false;
		SOCKET receiveSocket = getInvalidSocketValue();
		kt::InternetProtocolVersion protocolVersion = kt::InternetProtocolVersion::Any;
		unsigned int listeningPort = 0;

		std::pair<int, kt::SocketAddress> constructSocket(std::string&, unsigned int&, const kt::InternetProtocolVersion = kt::InternetProtocolVersion::Any);
		int pollSocket(SOCKET socket, const long& = 1000) const;
		void initialiseListeningPortNumber();

		void close(SOCKET socket);

	public:
		UDPSocket() = default;
		UDPSocket(const kt::UDPSocket&);
		kt::UDPSocket& operator=(const kt::UDPSocket&);

		bool bind(const unsigned int&, const kt::InternetProtocolVersion = kt::InternetProtocolVersion::Any);
		void close();

		bool ready(const unsigned long = 1000) const;
		std::pair<bool, int> sendTo(const std::string&, const kt::SocketAddress&, const int& = 0);
		std::pair<bool, std::pair<int, kt::SocketAddress>> sendTo(const std::string&, const unsigned int&, const std::string&, const int& = 0);
		std::pair<std::optional<std::string>, kt::SocketAddress>  receiveFrom(const unsigned int&, const int& = 0);

		bool isUdpBound() const;
		kt::InternetProtocolVersion getInternetProtocolVersion() const;
		unsigned int getListeningPort() const;
	};

} // End namespace kt 
