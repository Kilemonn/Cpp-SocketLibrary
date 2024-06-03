
#ifndef _SERVER_SOCKET_H__
#define _SERVER_SOCKET_H__

#include <optional>

#include "../socket/Socket.h"

#include "../enums/SocketProtocol.h"
#include "../enums/SocketType.h"
#include "../enums/InternetProtocolVersion.h"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define _WIN32_WINNT 0x501

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
		private:
			unsigned int port;
			SocketType type = SocketType::None;
			InternetProtocolVersion protocolVersion = InternetProtocolVersion::IPV4;
			sockaddr serverAddress;
			SOCKET socketDescriptor = 0;

			void setDiscoverable();
			void constructSocket(const unsigned int&);
			void constructBluetoothSocket(const unsigned int&);
			void constructWifiSocket(const unsigned int&);
			void initialisePortNumber();
			void initialiseServerAddress(const int&);

		public:
			ServerSocket() = default;
			ServerSocket(const SocketType, const unsigned int& = 0, const unsigned int& = 20, const InternetProtocolVersion = InternetProtocolVersion::IPV4);
			ServerSocket(const ServerSocket&);
			ServerSocket& operator=(const ServerSocket&);

			SocketType getType() const;
			InternetProtocolVersion getInternetProtocolVersion() const;
			unsigned int getPort() const;

			Socket accept(const unsigned int& = 0);
			void close();
	};

} // End namespace kt

#endif // _SERVER_SOCKET_H__
