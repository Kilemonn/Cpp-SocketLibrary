
#include "Socket.h"
#include "../SocketExceptions/SocketError.hpp"

#include <iostream>
#include <vector>
#include <utility>

#ifdef _WIN32

#include <windows.h>

#elif __linux__

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#endif


// Throws SocketError when can't connect to server
Socket::Socket(const std::string& hostname, const int& port, const bool isWifi)
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

Socket::Socket(const int& socketDescriptor, const bool isWifi)
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

Socket& Socket::operator=(const Socket& socket)
{
	this->socketDescriptor = socket.socketDescriptor;
	this->hostname = socket.hostname;
	this->port = socket.port;
	this->isWifi = socket.isWifi;

	this->bluetoothAddress = socket.bluetoothAddress;
	this->serverAddress = socket.serverAddress;

	return *this;
}

void Socket::constructBluetoothSocket()
{
	socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	if (socketDescriptor != 0)
    {
    	throw SocketError("Error establishing Bluetooth socket...");
    }

    bluetoothAddress.rc_family = AF_BLUETOOTH;
    bluetoothAddress.rc_channel = (uint8_t) port;
    str2ba(this->hostname.c_str(), &bluetoothAddress.rc_bdaddr);

   	if (connect(socketDescriptor, (struct sockaddr *) &bluetoothAddress, sizeof(bluetoothAddress)) == -1)
   	{
   		throw SocketError("Error connecting to Bluetooth server");
	}
}

void Socket::constructWifiSocket()
{
	struct hostent* server = gethostbyname(this->hostname.c_str());
    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (socketDescriptor == -1)
    {
    	throw SocketError("Error establishing Wifi socket...");
    }

    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(this->port);

	if (connect(socketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
	{
		throw SocketError("Error connecting to Wifi server");
	}
}

void Socket::closeSocket()
{
	close(socketDescriptor);
}

bool Socket::sendMessage(const std::string message, int serverity) const
{
	if (send(socketDescriptor, message.c_str(), message.size(), serverity) == -1) 
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool Socket::ready() const
{
	fd_set sready;
	struct timeval nowait;

	FD_ZERO(&sready);
	FD_SET((unsigned int)this->socketDescriptor, &sready);
	memset((char*) &nowait, 0, sizeof(nowait));

	select(this->socketDescriptor + 1, &sready, nullptr, nullptr, &nowait);
	if (FD_ISSET(this->socketDescriptor, &sready))
	{
		return true;
	}
	else
	{
		return false;
	}
}

char Socket::get() const
{
	return this->receiveAmount(1)[0];
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

std::vector<std::pair<std::string, std::string> > Socket::scanDevices()
{
	std::vector<std::pair<std::string, std::string> > devices;
	std::pair<std::string, std::string> tempPair;

	inquiry_info *ii = nullptr;
    int maxResponse = 255, len = 8, numberOfResponses, ownId, tempSocket, flags;
    char deviceAddress[19];
    char deviceName[248];

    ownId = hci_get_route(nullptr);
    tempSocket = hci_open_dev( ownId );
    if (ownId < 0 || tempSocket < 0)
    {
        throw SocketError("Error opening Bluetooth socket for scanning...");
    }

    flags = IREQ_CACHE_FLUSH;
    ii = new inquiry_info[maxResponse * sizeof(inquiry_info)];
    
    numberOfResponses = hci_inquiry(ownId, len, maxResponse, nullptr, &ii, flags);
    if( numberOfResponses < 0 )
    {
    	delete ii;
    	throw SocketError("Error scanning for bluetooth devices");
    }

    for (int i = 0; i < numberOfResponses; i++) 
    {
        ba2str( &(ii + i)->bdaddr, deviceAddress);
        bzero(&deviceName, sizeof(deviceName));
        if (hci_read_remote_name(tempSocket, &(ii+i)->bdaddr, sizeof(deviceName), deviceName, 0) < 0)
        {
        	strcpy(deviceName, "[unknown]");
        }

        tempPair = std::make_pair<std::string, std::string>(deviceAddress, deviceName);
        devices.push_back(tempPair);

    }

    delete ii;
    close( tempSocket );

	return std::move(devices);
}
