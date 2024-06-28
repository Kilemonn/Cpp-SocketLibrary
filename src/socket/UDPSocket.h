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
{
	class UDPSocket
	{
	protected:
		bool bound = false;
		SOCKET receiveSocket = getInvalidSocketValue();
		kt::InternetProtocolVersion protocolVersion = kt::InternetProtocolVersion::Any;
		std::optional<unsigned int> listeningPort = std::nullopt;

		int pollSocket(SOCKET socket, const long& = 1000) const;
		void initialiseListeningPortNumber();
		void close(SOCKET socket);

	public:
		UDPSocket() = default;
		UDPSocket(const kt::UDPSocket&);
		kt::UDPSocket& operator=(const kt::UDPSocket&);

		bool bind(const unsigned int& = 0, const kt::InternetProtocolVersion = kt::InternetProtocolVersion::Any);
		void close();
		bool ready(const unsigned long = 1000) const;

		std::pair<bool, int> sendTo(const std::string&, const kt::SocketAddress&, const int& = 0);
		std::pair<bool, int> sendTo(const char*, const int&, const kt::SocketAddress&, const int& = 0);
		std::pair<bool, std::pair<int, kt::SocketAddress>> sendTo(const std::string&, const unsigned int&, const std::string&, const int& = 0);
		std::pair<bool, std::pair<int, kt::SocketAddress>> sendTo(const std::string&, const unsigned int&, const char*, const int&, const int& = 0);
		
		std::pair<std::optional<std::string>, std::pair<int, kt::SocketAddress>> receiveFrom(const int&, const int& = 0);
		std::pair<int, kt::SocketAddress> receiveFrom(char*, const int&, const int& = 0) const;

		bool isUdpBound() const;
		kt::InternetProtocolVersion getInternetProtocolVersion() const;
		std::optional<unsigned int> getListeningPort() const;
	};

} // End namespace kt 
