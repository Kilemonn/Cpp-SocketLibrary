
#ifndef _SERVER_SOCKET_H__
#define _SERVER_SOCKET_H__

#include "../Socket/Socket.h"

#ifdef _WIN32

#include <windows.h>

#elif __linux__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#endif

class ServerSocket
{
	private:
		int socketDescriptor;
		unsigned int port;
		bool isWifi;
		struct sockaddr_in serverAddress;
    	socklen_t socketSize;

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
