
#ifndef _SERVER_SOCKET_H__
#define _SERVER_SOCKET_H__

#include "../Socket/Socket.h"

#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class ServerSocket
{
	private:
		int socketDescriptor, port;
		bool isWifi;
		struct sockaddr_in serverAddress;
    	socklen_t socketSize;

    	void constructBluetoothSocket();
		void constructWifiSocket();

	public:
		ServerSocket(const int, const bool);
		ServerSocket(const ServerSocket&);
		ServerSocket& operator=(const ServerSocket&);

		Socket acceptWifiConnection();
		Socket acceptBluetoothConnection();
};

#endif // _SERVER_SOCKET_H__
