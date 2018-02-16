
#ifndef _SERVER_SOCKET_H__
#define _SERVER_SOCKET_H__

#include "Socket.h"

#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class ServerSocket
{
	private:
		int socketDescriptor, port;
		struct sockaddr_in serverAddress;
    	socklen_t socketSize;

    	void doConnect();

	public:
		ServerSocket(const int);
		Socket acceptConnection();
};

#endif // _SERVER_SOCKET_H__
