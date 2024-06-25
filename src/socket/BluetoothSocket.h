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
	class BluetoothSocket
	{
	protected:
		std::string hostname;
		unsigned int port;
		SOCKET socketDescriptor;

#ifdef _WIN32
		//SOCKADDR_BTH bluetoothAddress;

#elif __linux__
		sockaddr_rc bluetoothAddress; // For Bluetooth

#endif

		void constructBluetoothSocket();
		int pollSocket(SOCKET socket, const long& = 1000) const;

		void close(SOCKET socket);

	public:
		BluetoothSocket() = default;
		BluetoothSocket(const std::string&, const unsigned int&);
		//BluetoothSocket(const SOCKET&, const kt::SocketType, const kt::SocketProtocol, const std::string&, const unsigned int&, const kt::InternetProtocolVersion);

		//BluetoothSocket(const kt::BluetoothSocket&);
		//kt::BluetoothSocket& operator=(const kt::BluetoothSocket&);

		void close();

		//bool ready(const unsigned long = 1000) const;
		//bool connected(const unsigned long = 1000) const;
		bool send(const std::string&, int = 0);

		unsigned int getPort() const;
		std::string getHostname() const;

		std::optional<char> get(const int&) const;
		std::string receiveAmount(const unsigned int&, const int& = 0) const;
		//std::string receiveToDelimiter(const char&, unsigned int = 0);
		//std::string receiveAll(const unsigned long = 1000);

		static std::vector<std::pair<std::string, std::string> > scanDevices(unsigned int = 5);
		static std::optional<std::string> getLocalMACAddress();
	};

} // End namespace kt 
