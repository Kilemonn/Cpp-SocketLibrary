
#include "Socket.h"

#include <cstring>
#include <iostream>

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>


Socket::Socket(const std::string hostname, const int port, const bool isWifi)
{
	this->isWifi = isWifi;
	this->hostname = hostname;
	this->port = port;

	if (isWifi)
	{
		this->constructWifiSocket();
	}
	else
	{
		this->constructBluetoothSocket();
	}
}

Socket::Socket(const int socketDescriptor, const bool isWifi)
{
	this->socketDescriptor = socketDescriptor;
	this->hostname = "";
	this->port = 0;
	this->isWifi = isWifi;
}

Socket::Socket(const Socket& socket)
{
	this->socketDescriptor = socket.socketDescriptor;
	this->hostname = socket.hostname;
	this->port = socket.port;
	this->isWifi = socket.isWifi;

	this->bluetoothAddress = socket.bluetoothAddress;
	this->serverAddress = socket.serverAddress;
}

void Socket::constructBluetoothSocket()
{
	socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    bluetoothAddress.rc_family = AF_BLUETOOTH;
    bluetoothAddress.rc_channel = (uint8_t) port;
    str2ba(this->hostname.c_str(), &bluetoothAddress.rc_bdaddr);

   	connect(socketDescriptor, (struct sockaddr *) &bluetoothAddress, sizeof(bluetoothAddress));
}

void Socket::constructWifiSocket()
{
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

void Socket::closeSocket()
{
	close(socketDescriptor);
}

bool Socket::sendMessage(const std::string message, int serverity) const
{
	if (isWifi)
	{
		if (send(socketDescriptor, message.c_str(), message.size(), serverity) == -1) 
		{
			return false;
		}
	}
	else
	{
		if (write(socketDescriptor, message.c_str(), message.size()) == -1)
		{
			return false;
		}
	}
	return true;
}

std::string Socket::receiveAmount(const int amountToReceive) const
{
	char data[amountToReceive];
	int counter = 0, flag = 0;
	std::string result = "";

	while (counter < amountToReceive)
	{
		bzero(&data, amountToReceive);
		if (isWifi)
		{
			flag = recv(socketDescriptor, data, (amountToReceive - counter), 0);
		}
		else
		{
			flag = read(socketDescriptor, data, (amountToReceive - counter));
		}
		
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
		if (isWifi)
		{
			flag = recv(socketDescriptor, temp, 1, 0);
		}
		else
		{
			flag = read(socketDescriptor, temp, 1);
		}
		
		if (flag == 0)
		{
			return data;
		}
		data += temp[0];
	} while (temp[0] != delimiter);

	return data.substr(0, (data.size() - 1));
}
