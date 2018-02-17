
#include "Socket.h"

#include <cstring>
#include <iostream>

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

Socket::Socket(const std::string hostname, const int port)
{
	this->hostname = hostname;
	this->port = port;

	// Construct socket
	struct hostent* server = gethostbyname(this->hostname.c_str());
    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (socketDescriptor < 0)
    {
    	std::cerr << "Error establishing socket..." << std::endl;
        return;
    }

    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(this->port);

	connect(socketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
}

Socket::Socket(const int socketDescriptor)
{
	this->socketDescriptor = socketDescriptor;
	this->hostname = "";
	this->port = 0;
}

Socket::Socket(const Socket& socket)
{
	this->hostname = socket.hostname;
	this->port = socket.port;
	this->socketDescriptor = socket.socketDescriptor;

	this->serverAddress = socket.serverAddress;
}

void Socket::closeSocket()
{
	close(socketDescriptor);
}

void Socket::sendMessage(const std::string message, int serverity) const
{
	send(socketDescriptor, message.c_str(), message.size(), serverity);
}

std::string Socket::receiveAmount(const int amountToReceive) const
{
	char data[amountToReceive];
	int counter = 0, flag = 0;
	std::string result = "";

	while (counter < amountToReceive)
	{
		bzero(&data, amountToReceive);
		flag = recv(socketDescriptor, data, (amountToReceive - counter), 0);
		if (flag == 0)
		{
			return std::string(result);
		}
		result += std::string(data);
		counter += flag;
	}
	return std::string(result);
}

std::string Socket::receiveToDelimiter(const char delimiter) const
{
	std::string data = "";
	char temp[2];
	int flag;

	do
	{
		bzero(&temp, sizeof(temp));
		flag = recv(socketDescriptor, temp, 1, 0);

		if (flag == 0)
		{
			return data;
		}
		data += temp[0];
	} while (temp[0] != delimiter);

	return data.substr(0, (data.size() - 1));
}
