

#ifndef _SOCKET_H__
#define _SOCKET_H__

#include <iostream>
#include <vector>
#include <utility>

#include "../Enums/SocketProtocol.cpp"
#include "../Enums/SocketType.cpp"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define _WIN32_WINNT 0x501

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#include "../includes/guiddef.h"
#include "../includes/ws2bth.h"

#elif __linux__

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>

#endif // _WIN32 / __linux__

namespace kt
{
	class Socket
	{
		private:
			std::string hostname;
			unsigned int port;
			kt::SocketProtocol protocol;
			kt::SocketType type;
			struct sockaddr_in clientAddress; // For UDP, stores the client address of the last message received

#ifdef _WIN32

			// Wifi Properties
			struct addrinfo* serverAddress;
	        struct addrinfo hints;
	    	SOCKET socketDescriptor;

	    	SOCKADDR_BTH bluetoothAddress;

#elif __linux__

			int socketDescriptor;
			struct sockaddr_in serverAddress; // For Wifi
			struct sockaddr_rc bluetoothAddress; // For Bluetooth

#endif // _WIN32 / __linux__

			void constructBluetoothSocket();
			void constructWifiSocket();
			int pollSocket(const unsigned long) const;

		public:
			Socket(const std::string&, const kt::SocketType, const unsigned int&,  const kt::SocketProtocol = kt::SocketProtocol::None); // Create Wi-Fi/Bluetooth Socket

#ifdef _WIN32

			Socket(const SOCKET&, const kt::SocketType, const kt::SocketProtocol, const std::string&, const unsigned int&);

#elif __linux__

			Socket(const int&, const kt::SocketType, const kt::SocketProtocol, const std::string&, const unsigned int&);

#endif

			Socket(const Socket&); // Copy Constructor
			Socket& operator=(const Socket&);
			
			bool bind();
			bool unbind();
			void close();
			bool ready(const unsigned long = 1) const;
			bool connected(const unsigned long = 1) const;
			bool send(const std::string&, int = 0) const;
			bool sendTo(const std::string&, const std::string&, int = 0);
			char get() const;
			int getPort() const;
			std::string getLastRecievedAddress() const;
			std::string getAddress() const;
			std::string receiveAmount(const unsigned int) const;
			std::string receiveToDelimiter(const char) const;
			std::string receiveAll(const unsigned long = 10000) const;

			static std::vector<std::pair<std::string, std::string> > scanDevices(unsigned int = 5);
			static std::string getLocalMACAddress();
	};

} // End namespace kt 

#endif //_SOCKET_H__
