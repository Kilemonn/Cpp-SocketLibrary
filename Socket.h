

#ifndef _SOCKET_H__
#define _SOCKET_H__

#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

class Socket
{
	private:
		std::string hostname;
		int socketDescriptor, port;
		struct sockaddr_in serverAddress;

    	void doConnect();

	public:
		Socket(const std::string, const int);
		Socket(const int);
		Socket(const Socket&);
		void closeSocket();
		void sendMessage(const std::string, int severity = 0) const;
		std::string receiveAmount(int) const;
		std::string receiveToDelimiter(const char) const;
};

#endif //_SOCKET_H__
