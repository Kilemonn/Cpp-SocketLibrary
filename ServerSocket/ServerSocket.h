
#ifndef _SERVER_SOCKET_H__
#define _SERVER_SOCKET_H__

#include "../Socket/Socket.h"

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

    	void constructSocket();
    	void constructBluetoothSocket();
		void constructWifiSocket();

	public:
		const static bool WIFI = true;
		const static bool BLUETOOTH = false;

		ServerSocket(const bool, const int = 0);
		ServerSocket(const ServerSocket&);
		ServerSocket& operator=(const ServerSocket&);

		Socket acceptConnection();
		int getPort() const;
};

#endif // _SERVER_SOCKET_H__
