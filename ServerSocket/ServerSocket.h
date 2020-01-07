
#ifndef _SERVER_SOCKET_H__
#define _SERVER_SOCKET_H__

#include "../Socket/Socket.h"

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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#endif

namespace kt
{
	class ServerSocket
	{
		private:
			unsigned int port;
			kt::SocketType type;

#ifdef _WIN32

			// Wifi properties
			struct addrinfo *serverAddress;
	        struct addrinfo hints;
	        SOCKADDR_BTH bluetoothAddress;
			SOCKET socketDescriptor;

#elif __linux__

			int socketDescriptor;
			struct sockaddr_in serverAddress;
	    	socklen_t socketSize;

#endif

			void setDiscoverable();
			void constructSocket(const unsigned int&);
			void constructBluetoothSocket(const unsigned int&);
			void constructWifiSocket(const unsigned int&);
			void randomlyAllocatePort(const unsigned int&);

		public:
			ServerSocket(const kt::SocketType, const unsigned int& = 0, const unsigned int& = 20);
			ServerSocket(const ServerSocket&);
			ServerSocket& operator=(const ServerSocket&);

			Socket accept(const unsigned int& = 0);
			unsigned int getPort() const;
			void close();
	};

} // End namespace kt

#endif // _SERVER_SOCKET_H__
