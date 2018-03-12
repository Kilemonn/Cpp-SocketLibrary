
#ifndef _SERVER_SOCKET_H__
#define _SERVER_SOCKET_H__

#include "../Socket/Socket.h"

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

class ServerSocket
{
	private:
		unsigned int port;
		bool isWifi;

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

    	void constructSocket();
    	void constructBluetoothSocket();
		void constructWifiSocket();

	public:
		const static bool WIFI = true;
		const static bool BLUETOOTH = false;

		ServerSocket(const bool, const unsigned int& = 0);
		ServerSocket(const ServerSocket&);
		ServerSocket& operator=(const ServerSocket&);
		~ServerSocket();

		Socket accept();
		unsigned int getPort() const;
		void close();
};

#endif // _SERVER_SOCKET_H__
