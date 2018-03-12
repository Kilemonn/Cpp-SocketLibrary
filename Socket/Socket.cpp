
#include "Socket.h"
#include "../SocketExceptions/SocketError.hpp"

#include <iostream>
#include <vector>
#include <utility>
#include <string>

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
#include <unistd.h>
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#endif

#ifdef _WIN32

Socket::Socket(const std::string& hostname, const unsigned int& port, const bool isWifi)
{
	this->hostname = hostname;
	this->port = port;
	this->isWifi = isWifi;
	this->serverAddress = nullptr;
	this->socketDescriptor = INVALID_SOCKET;

	WSADATA wsaData;
	int res = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (res != 0)
    {
    	throw SocketError("WSAStartup Failed. " + std::to_string(res));
    }

    if (isWifi)
    {
    	if (this->hostname == "")
		{
			this->hostname = "127.0.0.1";
		}
		this->constructWifiSocket();
    }
    else
    {
    	if (this->hostname == "")
		{
			// this->hostname = "" - loop back for bluetooth?
		}
		this->constructBluetoothSocket();
    }
}

#elif __linux__
// Throws SocketError when can't connect to server
Socket::Socket(const std::string& hostname, const unsigned int& port, const bool isWifi)
{
	this->hostname = hostname;
	this->port = port;
	this->isWifi = isWifi;
	this->serverAddress = { 0 };
	this->bluetoothAddress = { 0 };

	if (isWifi)
	{
		if (this->hostname == "")
		{
			this->hostname = "127.0.0.1";
		}
		this->constructWifiSocket();
	}
	else
	{
		if (this->hostname == "")
		{
			// this->hostname = "" - loop back for bluetooth?
		}
		this->constructBluetoothSocket();
	}
}

#endif

#ifdef _WIN32

Socket::Socket(const SOCKET& socketDescriptor, const bool isWifi)
{
	this->socketDescriptor = socketDescriptor;
	this->isWifi = isWifi;
}

#elif __linux__

Socket::Socket(const int& socketDescriptor, const bool isWifi)
{
	this->socketDescriptor = socketDescriptor;
	this->isWifi = isWifi;
}

#endif

#ifdef _WIN32

Socket::Socket(const Socket& socket)
{
	this->socketDescriptor = socket.socketDescriptor;
	this->hostname = socket.hostname;
	this->port = socket.port;
	this->isWifi = socket.isWifi;

	this->serverAddress = socket.serverAddress;
}

#elif __linux__

Socket::Socket(const Socket& socket)
{
	this->socketDescriptor = socket.socketDescriptor;
	this->hostname = socket.hostname;
	this->port = socket.port;
	this->isWifi = socket.isWifi;

	this->bluetoothAddress = socket.bluetoothAddress;
	this->serverAddress = socket.serverAddress;
}

#endif

#ifdef _WIN32

Socket& Socket::operator=(const Socket& socket)
{
	this->close();

	this->socketDescriptor = socket.socketDescriptor;
	this->hostname = socket.hostname;
	this->port = socket.port;
	this->isWifi = socket.isWifi;

	this->serverAddress = socket.serverAddress;

	return *this;
}

#elif __linux__

Socket& Socket::operator=(const Socket& socket)
{
	this->close();

	this->socketDescriptor = socket.socketDescriptor;
	this->hostname = socket.hostname;
	this->port = socket.port;
	this->isWifi = socket.isWifi;

	this->bluetoothAddress = socket.bluetoothAddress;
	this->serverAddress = socket.serverAddress;

	return *this;
}

#endif

Socket::~Socket()
{
	#ifdef _WIN32

	// WSACleanup();
	freeaddrinfo(serverAddress);

	#endif

	this->close();
}


#ifdef _WIN32

void Socket::constructBluetoothSocket()
{
	// throw SocketError("Bluetooth sockets are not supported in Windows.");

	socketDescriptor = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

	if (socketDescriptor == INVALID_SOCKET)
	{
		throw SocketError("Error establishing Bluetooth socket...");
	}

	bluetoothAddress.addressFamily = AF_BTH;
    bluetoothAddress.btAddr = std::stoull(this->hostname);
    bluetoothAddress.port = this->port;

    if (connect(socketDescriptor, (struct sockaddr *) &bluetoothAddress, sizeof(SOCKADDR_BTH)) == SOCKET_ERROR)
    {
    	throw SocketError("Error connecting to Bluetooth server");
    }
}

#elif __linux__

void Socket::constructBluetoothSocket()
{
	socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	if (socketDescriptor == -1)
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

#endif

#ifdef _WIN32

void Socket::constructWifiSocket()
{
	int res;
	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	res = getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &serverAddress);

	if (res != 0) 
	{
	    throw SocketError("Unable to retrieving host address.");
	}

	socketDescriptor = socket(serverAddress->ai_family, serverAddress->ai_socktype, serverAddress->ai_protocol);
	if (socketDescriptor == INVALID_SOCKET) 
	{
	    throw SocketError("Error establishing Wifi socket.");
	}

    res = connect(socketDescriptor, serverAddress->ai_addr, (int)serverAddress->ai_addrlen);
	if(res == SOCKET_ERROR)
	{
		throw SocketError("Error connecting to Wifi server.");
	}
}

#elif __linux__

void Socket::constructWifiSocket()
{
	struct hostent* server = gethostbyname(this->hostname.c_str());
    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (socketDescriptor == -1)
    {
    	throw SocketError("Error establishing Wifi socket.");
    }

    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(this->port);

	if (connect(socketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
	{
		throw SocketError("Error connecting to Wifi server.");
	}
}

#endif

void Socket::close()
{
	#ifdef _WIN32

	closesocket(socketDescriptor);

	#elif __linux__

	::close(socketDescriptor);

	#endif
}

bool Socket::send(const std::string message, int flag) const
{
	if (::send(socketDescriptor, message.c_str(), message.size(), flag) == -1) 
	{
		return false;
	}
	else
	{
		return true;
	}
}


#ifdef _WIN32

bool Socket::ready() const
{
	throw SocketError("Socket::ready() is not supported on windows");
}

#elif __linux__

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

#endif

char Socket::get() const
{
	return this->receiveAmount(1)[0];
}

std::string Socket::receiveAmount(const unsigned int amountToReceive) const
{
	char data[(amountToReceive + 1)];
	unsigned int counter = 0;
	int flag = 0;
	std::string result;
	result.reserve(amountToReceive);

	while (counter < amountToReceive)
	{
		#ifdef _WIN32

		ZeroMemory (&data, (amountToReceive + 1));

		#elif __linux__

		bzero(&data, (amountToReceive + 1));

		#endif

		flag = recv(socketDescriptor, data, (amountToReceive - counter), 0);
		
		if (flag == -1)
		{
			return std::string(result);
		}
		result += std::string(data);
		counter += flag;
	}
	return std::string(result);
}

// Do not pass in '\0' as a delimiter
std::string Socket::receiveToDelimiter(const char delimiter) const
{
	std::string data = "";
	char temp[2];
	int flag;

	if (delimiter == '\0')
	{
		return data;
	}

	do
	{
		#ifdef _WIN32

		ZeroMemory (&temp, sizeof(temp));

		#elif __linux__

		bzero(&temp, sizeof(temp));

		#endif
			
		flag = recv(socketDescriptor, temp, 1, 0);

		if (flag == -1)
		{
			return data;
		}
		data += temp[0];
	} while (temp[0] != delimiter);

	return data.substr(0, (data.size() - 1));
}

#ifdef _WIN32

std::vector<std::pair<std::string, std::string> > Socket::scanDevices(unsigned int duration)
{
	throw SocketError("Not supported for Windows platform yet.");
}

#elif __linux__

std::vector<std::pair<std::string, std::string> > Socket::scanDevices(unsigned int duration)
{
	std::vector<std::pair<std::string, std::string> > devices;
	std::pair<std::string, std::string> tempPair;

	inquiry_info *ii = nullptr;
    int maxResponse = 255, numberOfResponses, ownId, tempSocket, flags;
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
    
    numberOfResponses = hci_inquiry(ownId, duration, maxResponse, nullptr, &ii, flags);
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
    ::close( tempSocket );

	return std::move(devices);
}

#endif
