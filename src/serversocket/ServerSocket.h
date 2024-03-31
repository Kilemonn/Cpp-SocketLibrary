
#ifndef _SERVER_SOCKET_H__
#define _SERVER_SOCKET_H__

#include <optional>

#include "../socket/Socket.h"

#include "../enums/SocketProtocol.cpp"
#include "../enums/SocketType.cpp"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define _WIN32_WINNT 0x501

#include <WinSock2.h>
#include <ws2bth.h>

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
			kt::SocketType type = SocketType::None;
			int socketDescriptor = 0;

#ifdef _WIN32
			struct addrinfo* serverAddress;
			struct addrinfo hints;
			// SOCKADDR_BTH bluetoothAddress;

#elif __linux__
			struct sockaddr_in serverAddress;
	    	socklen_t socketSize;

#endif

			void setDiscoverable();
			void constructSocket(const unsigned int&);
			void constructBluetoothSocket(const unsigned int&);
			void constructWifiSocket(const unsigned int&);

		public:
			ServerSocket() = default;
			ServerSocket(const kt::SocketType, const unsigned int& = 0, const unsigned int& = 20);
			ServerSocket(const ServerSocket&);
			ServerSocket& operator=(const ServerSocket&);

			kt::SocketType getType() const;
			unsigned int getPort() const;

			Socket accept(const unsigned int& = 0);
			void close();
	};

} // End namespace kt

#endif // _SERVER_SOCKET_H__
