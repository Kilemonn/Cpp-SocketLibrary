#pragma once

#include <iostream>
#include <vector>
#include <utility>
#include <optional>
#include <functional>

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
		std::optional<unsigned short> listeningPort = std::nullopt;
		std::optional<std::function<void(SOCKET&)>> preSendSocketOperation = std::nullopt;

		int pollSocket(SOCKET socket, const long& = 1000) const;
		void initialiseListeningPortNumber();
		void close(SOCKET socket);

	public:
		UDPSocket() = default;
		UDPSocket(const kt::UDPSocket&);
		kt::UDPSocket& operator=(const kt::UDPSocket&);

		std::pair<bool, kt::SocketAddress> bind(const std::optional<std::string>& = std::nullopt, const unsigned short& = 0, const kt::InternetProtocolVersion = kt::InternetProtocolVersion::Any, const std::optional<std::function<void(SOCKET&)>>& = std::nullopt);
		void close();
		bool ready(const unsigned long = 100) const;

		int sendTo(const std::string&, const kt::SocketAddress&, const int& = 0);
		int sendTo(const char*, const int&, const kt::SocketAddress&, const int& = 0);
		std::pair<int, kt::SocketAddress> sendTo(const std::string&, const unsigned short&, const std::string&, const int& = 0, const kt::InternetProtocolVersion = kt::InternetProtocolVersion::Any);
		std::pair<int, kt::SocketAddress> sendTo(const std::string&, const unsigned short&, const char*, const int&, const int& = 0, const kt::InternetProtocolVersion = kt::InternetProtocolVersion::Any);
		
		std::pair<std::optional<std::string>, std::pair<int, kt::SocketAddress>> receiveFrom(const int&, const int& = 0);
		std::pair<int, kt::SocketAddress> receiveFrom(char*, const int&, const int& = 0) const;

		SOCKET getListeningSocket() const;
		bool isUdpBound() const;
		kt::InternetProtocolVersion getInternetProtocolVersion() const;
		std::optional<unsigned short> getListeningPort() const;

		void setPreSendSocketOperation(std::function<void(SOCKET&)>);
	};

} // End namespace kt 
