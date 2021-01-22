
#ifndef _SERVER_SOCKET_H__
#define _SERVER_SOCKET_H__

#include "../socket/Socket.h"

#include "../enums/SocketProtocol.cpp"
#include "../enums/SocketType.cpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace kt
{
	class ServerSocket
	{
		private:
			unsigned int port;
			kt::SocketType type = SocketType::None;
			int socketDescriptor = 0;
			struct sockaddr_in serverAddress;
	    	socklen_t socketSize;

			void setDiscoverable();
			void constructSocket(const unsigned int&);
			void constructBluetoothSocket(const unsigned int&);
			void constructWifiSocket(const unsigned int&);
			void randomlyAllocatePort(const unsigned int&);

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
