
#include "Socket.h"
#include "../SocketExceptions/SocketException.hpp"
#include "../SocketExceptions/BindingException.hpp"
#include "../Enums/SocketProtocol.cpp"
#include "../Enums/SocketType.cpp"

#include <iostream>
#include <vector>
#include <utility>
#include <cstring>
#include <sstream>
#include <iomanip>

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
#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif

namespace kt
{

	Socket::Socket(const std::string& hostname, const kt::SocketType type, const unsigned int& port, const kt::SocketProtocol protocol, const unsigned int& receivePort)
	{
		this->hostname = hostname;
		this->port = port;
		this->type = type;
		this->protocol = protocol;
		this->receivePort = receivePort;
		this->serverAddress = { 0 };

#ifdef _WIN32

		this->socketDescriptor = INVALID_SOCKET;

		WSADATA wsaData;
		int res = WSAStartup(MAKEWORD(2,2), &wsaData);
	    if (res != 0)
	    {
	    	throw SocketException("WSAStartup Failed. " + std::to_string(res));
	    }

#elif __linux__

		this->bluetoothAddress = { 0 };

#endif

		if (this->type == kt::SocketType::Wifi && this->protocol == kt::SocketProtocol::None)
        {
            throw SocketException("Unable to set protocol to 'None' for a Wifi Socket.");
        }

	    if (type == kt::SocketType::Wifi)
	    {
			this->constructWifiSocket();
	    }
	    else
	    {
			this->constructBluetoothSocket();
	    }
	}

#ifdef _WIN32

	Socket::Socket(const SOCKET& socketDescriptor, const kt::SocketType type, const kt::SocketProtocol protocol, const std::string& hostname, const unsigned int& port)

#elif __linux__

Socket::Socket(const int& socketDescriptor, const kt::SocketType type, const kt::SocketProtocol protocol, const std::string& hostname, const unsigned int& port)

#endif

	{
		this->hostname = hostname;
		this->port = port;
		this->protocol = protocol;
		this->socketDescriptor = socketDescriptor;
		this->type = type;
	}

	Socket::Socket(const Socket& socket)
	{
		this->socketDescriptor = socket.socketDescriptor;
		this->hostname = socket.hostname;
		this->protocol = socket.protocol;
		this->port = socket.port;
		this->type = socket.type;

		this->serverAddress = socket.serverAddress;

#ifdef __linux__

		this->bluetoothAddress = socket.bluetoothAddress;

#endif
	}

	Socket& Socket::operator=(const Socket& socket)
	{
		this->socketDescriptor = socket.socketDescriptor;
		this->hostname = socket.hostname;
		this->protocol = socket.protocol;
		this->port = socket.port;
		this->type = socket.type;

		this->serverAddress = socket.serverAddress;

#ifdef __linux__

		this->bluetoothAddress = socket.bluetoothAddress;

#endif

		return *this;
	}

#ifdef _WIN32

	void Socket::constructBluetoothSocket()
	{
		// throw SocketException("Bluetooth sockets are not supported in Windows.");

		this->socketDescriptor = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

		if (this->socketDescriptor == INVALID_SOCKET)
		{
			throw SocketException("Error establishing Bluetooth socket: " + std::string(std::strerror(errno)));
		}

		this->bluetoothAddress.addressFamily = AF_BTH;
	    this->bluetoothAddress.btAddr = std::stoull(this->hostname);
	    this->bluetoothAddress.port = this->port;

	    if (connect(this->socketDescriptor, (struct sockaddr *) &this->bluetoothAddress, sizeof(SOCKADDR_BTH)) == SOCKET_ERROR)
	    {
	    	throw SocketException("Error connecting to Bluetooth server: " + std::string(std::strerror(errno)));
	    }
	}

#elif __linux__

	void Socket::constructBluetoothSocket()
	{
		this->socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

		if (this->socketDescriptor == -1)
	    {
	    	throw SocketException("Error establishing Bluetooth socket: " + std::string(std::strerror(errno)));
	    }

	    this->bluetoothAddress.rc_family = AF_BLUETOOTH;
	    this->bluetoothAddress.rc_channel = (uint8_t) port;
	    str2ba(this->hostname.c_str(), &this->bluetoothAddress.rc_bdaddr);

	   	if (connect(this->socketDescriptor, (struct sockaddr *) &this->bluetoothAddress, sizeof(this->bluetoothAddress)) == -1)
	   	{
	   		throw SocketException("Error connecting to Bluetooth server: " + std::string(std::strerror(errno)));
		}
	}
#endif

#ifdef _WIN32

	void Socket::constructWifiSocket()
	{
		memset(&this->hints, 0, sizeof(this->hints));
		this->hints.ai_family = AF_INET;
        int socketType = this->protocol == kt::SocketProtocol::TCP ? SOCK_STREAM : SOCK_DGRAM;
		this->hints.ai_socktype = socketType;
		int socketProtocol = this->protocol == kt::SocketProtocol::TCP ? IPPROTO_TCP : IPPROTO_UDP;
		this->hints.ai_protocol = socketProtocol;

		if (this->protocol == kt::SocketProtocol::TCP)
		{
			if (getaddrinfo(this->hostname.c_str(), std::to_string(this->port).c_str(), &this->hints, &this->serverAddress) != 0)
			{
				throw SocketException("Unable to retrieving host address: " + std::string(std::strerror(errno)));
			}

			this->socketDescriptor = socket(this->serverAddress->ai_family, this->serverAddress->ai_socktype, this->serverAddress->ai_protocol);
			if (this->socketDescriptor == INVALID_SOCKET) 
			{
				throw SocketException("Error establishing Wifi socket: " + std::string(std::strerror(errno)));
			}

			if (connect(this->socketDescriptor, this->serverAddress->ai_addr, (int)this->serverAddress->ai_addrlen) == SOCKET_ERROR)
			{
				throw SocketException("Error connecting to Wifi server: " + std::string(std::strerror(errno)));
			}
		}
		else if (this->protocol == kt::SocketProtocol::UDP)
		{
			this->socketDescriptor = socket(this->serverAddress->ai_family, this->serverAddress->ai_socktype, this->serverAddress->ai_protocol);
			if (this->socketDescriptor == INVALID_SOCKET) 
			{
				throw SocketException("Error establishing Wifi socket: " + std::string(std::strerror(errno)));
			}

			struct addrinfo* localAddress;
			struct addrinfo localHints;
			memset(&localHints, 0, sizeof(localHints));
			localHints.ai_family = AF_INET;
			localHints.ai_socktype = SOCK_STREAM;
			localHints.ai_protocol = IPPROTO_TCP;
			localHints.ai_flags = AI_PASSIVE;

			if (getaddrinfo(nullptr, std::to_string(this->receivePort).c_str(), &this->hints, &localAddress) != 0) 
			{
				throw SocketException("Unable to retrieving host address to self (localhost/127.0.0.1): " + std::string(std::strerror(errno)));
			}

			if (bind(this->socketDescriptor, localAddress->ai_addr, (int)localAddress->ai_addrlen) == SOCKET_ERROR)
			{
				throw BindingException("Error binding connection, the port " + std::to_string(this->receivePort) + " is already being used: " + std::string(std::strerror(errno)));
			}
		}
	}

#elif __linux__

	void Socket::constructWifiSocket()
	{
		int socketProtocol = this->protocol == kt::SocketProtocol::TCP ? SOCK_STREAM : SOCK_DGRAM;
	    this->socketDescriptor = socket(AF_INET, socketProtocol, 0);

	    if (this->socketDescriptor == -1)
	    {
	    	throw SocketException("Error establishing Wifi socket: " + std::string(std::strerror(errno)));
	    }

		struct hostent* server = gethostbyname(this->hostname.c_str());
		if (server != nullptr)
		{
			memset(&this->serverAddress, 0, sizeof(this->serverAddress));
			this->serverAddress.sin_family = AF_INET;
			bcopy((char *) server->h_addr, (char *) &this->serverAddress.sin_addr.s_addr, server->h_length);
			this->serverAddress.sin_port = htons(this->port);
		}
		else
		{
			if (this->protocol == kt::SocketProtocol::TCP)
			{
				throw SocketException("Error connecting to socket with IP: " + this->hostname + ".");
			}
		}

		if (this->protocol == kt::SocketProtocol::TCP)
		{
			if (connect(this->socketDescriptor, (struct sockaddr *)&this->serverAddress, sizeof(this->serverAddress)) == -1)
			{
				throw SocketException("Error connecting to Wifi server: " + std::string(std::strerror(errno)));
			}
		}
		else if (this->protocol == kt::SocketProtocol::UDP)
		{
			struct sockaddr_in localAddress;
			localAddress.sin_family = AF_INET;
			localAddress.sin_port = htons(this->receivePort);
			localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
			if (bind(this->socketDescriptor, (struct sockaddr*) &localAddress, sizeof(localAddress)) == -1) 
			{
				std::cout << "Error binding connection, the port " + std::to_string(this->receivePort) + " is already being used: " + std::string(std::strerror(errno)) << std::endl;
				// throw BindingException("Error binding connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)));
			}
		}
	}

#endif

	void Socket::close()
	{
#ifdef _WIN32

		freeaddrinfo(this->serverAddress);
		closesocket(this->socketDescriptor);

#elif __linux__

		::close(this->socketDescriptor);

#endif
	}

	bool Socket::send(const std::string& message, int flag) const
	{
		if (message.size() > 0)
		{
			if (this->protocol == kt::SocketProtocol::TCP)
			{
				if (::send(this->socketDescriptor, message.c_str(), message.size(), flag) == -1) 
				{
					return false;
				}
				else
				{
					return true;
				}
			}
		}
		return false;
	}

	bool Socket::sendTo(const std::string& address, const std::string& message, int flag)
	{
		if (message.size() > 0)
		{
			if (this->protocol == kt::SocketProtocol::UDP)
			{
				if (address.size() > 0 || this->clientAddress.sin_family == AF_UNSPEC)
				{
					memset(&this->clientAddress, 0, sizeof(this->clientAddress));

					struct hostent* client = gethostbyname(address.c_str());
					this->clientAddress.sin_family = AF_INET;
					memcpy((char *) client->h_addr, (char *) &this->clientAddress.sin_addr.s_addr, client->h_length);
					this->clientAddress.sin_port = htons(this->port);	
				}

				if (::sendto(this->socketDescriptor, message.c_str(), message.size(), flag, (const struct sockaddr *)&this->clientAddress, sizeof(this->clientAddress)) == -1)
				{
					return false;
				}
				else
				{
					return true;
				}
			}
		}
		return false;
	}

	bool Socket::ready(unsigned long timeout) const
	{
		fd_set sready;
		struct timeval timeOutVal;
		memset((char*) &timeOutVal, 0, sizeof(timeOutVal));
		timeOutVal.tv_usec = timeout;

		FD_ZERO(&sready);
		FD_SET((unsigned int)this->socketDescriptor, &sready);

		int res = select(this->socketDescriptor, &sready, nullptr, nullptr, &timeOutVal);
		if (FD_ISSET(this->socketDescriptor, &sready) || res > 0)
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

	int Socket::getPort() const
	{
		return this->port;
	}

	std::string Socket::getLastRecievedAddress() const
	{
		struct sockaddr_in address;
		socklen_t addr_size = sizeof(struct sockaddr_in);
		int res = getpeername(this->socketDescriptor, (struct sockaddr *)&address, &addr_size);

		std::string receivedAddress = "";
		if (res == 0)
		{ 
			char ip[20];
    		strcpy(ip, inet_ntoa(this->clientAddress.sin_addr));
			receivedAddress = std::string(ip);
		}
		return receivedAddress;
	}

	std::string Socket::getAddress() const
	{
		return this->hostname;
	}

	std::string Socket::receiveAmount(const unsigned int amountToReceive) const
	{
		if (amountToReceive == 0)
		{
			return "";
		}
		const unsigned int bufferSize = amountToReceive + 1;

		char data[bufferSize];
		unsigned int counter = 0;
		int flag = 0;
		std::string result;
		result.reserve(amountToReceive);

		while (counter < amountToReceive)
		{
			memset(&data, 0, bufferSize);

			if (this->protocol == kt::SocketProtocol::TCP)
			{
				flag = recv(this->socketDescriptor, data, (amountToReceive - counter), 0);
			}
			else if (this->protocol == kt::SocketProtocol::UDP)
			{
				socklen_t addressLength = sizeof(this->clientAddress);
				flag = recvfrom(this->socketDescriptor, data, (amountToReceive - counter), 0, (struct sockaddr*)&this->clientAddress, &addressLength);
			}
			
			if (flag < 1)
			{
				return std::move(result);
			}
			result += std::string(data);
			counter += flag;
		}
		return std::move(result);
	}

	// Do not pass in '\0' as a delimiter
	std::string Socket::receiveToDelimiter(const char delimiter) const
	{
		if (delimiter == '\0')
		{
			throw SocketException("The null terminator '\\0' is an invalid delimiter.");
		}

		std::string data = "";
		char temp[2];
		int flag;

		do
		{
			memset(&temp, 0, sizeof(temp));

			if (this->protocol == kt::SocketProtocol::TCP)
			{
				flag = recv(this->socketDescriptor, temp, 1, 0);
			}
			else if (this->protocol == kt::SocketProtocol::UDP)
			{
				socklen_t addressLength = sizeof(this->clientAddress);
				flag = recvfrom(this->socketDescriptor, temp, 1, 0, (struct sockaddr*)&this->clientAddress, &addressLength);
			}

			if (flag < 1)
			{
				return std::move(data);
			}
			data += temp[0];

		} while (temp[0] != delimiter);

		return std::move(data.substr(0, (data.size() - 1)));
	}

	std::string Socket::receiveAll(const unsigned long oneHundredMS) const
	{
		// Default is 100 milli seconds in micro seconds
		std::string result = "";
		result.reserve(1024);

		while (this->ready(oneHundredMS))
		{
			result += this->get();
		}
		return std::move(result);
	}

#ifdef _WIN32

	std::vector<std::pair<std::string, std::string> > Socket::scanDevices(unsigned int duration)
	{
 		WSADATA wsaData;
        int res = WSAStartup(MAKEWORD(2,2), &wsaData);
        if(res != 0)
        {
            throw SocketException("WSAStartup Failed: " + std::to_string(res));
        }

		throw SocketException("Not yet implemented on Windows.");
		
		/*WSAQUERYSET wsaQuery;
		HANDLE hLoopUp;
		LPWSAQUERYSET pQuerySet = nullptr;
		SOCKADDR_BTH tempAddress;
		DWORD dwSize = 5000 * sizeof(unsigned char);

		memset(&wsaQuery, 0, sizeof(WSAQUERYSET));
		wsaQuery.dwSize = sizeof(WSAQUERYSET);
		wsaQuery.dwNameSpace = NS_BTH;
		wsaQuery.lpcsaBuffer = nullptr;

		int res = WSALookupServiceBegin(&wsaQuery, LUP_CONTAINERS, &hLoopUp);

		if (res == SOCKET_ERROR)
		{
			throw SocketException("Unable to search for devices. Could not begin search.");
		}
		
		memset(&pQuerySet, 0, sizeof(WSAQUERYSET));
		pQuerySet->dwSize = sizeof(WSAQUERYSET);
		pQuerySet->dwNameSpace = NS_BTH;
		pQuerySet->lpBlob = nullptr;

		while (WSALookupServiceNext(hLoopUp, LUP_RETURN_NAME | LUP_RETURN_ADDR, &dwSize, pQuerySet) == 0)
		{
			tempAddress = ((SOCKADDR_BTH*) pQuerySet->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr;

			// std::cout << pQuerySet->lpszServiceInstanceName << " : " << GET_NAP(tempAddress) << " - " << GET_SAP(tempAddress) << " ~ " << pQuerySet->dwNameSpace << std::endl;
		}*/
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
	        throw SocketException("Error opening Bluetooth socket for scanning...");
	    }

	    flags = IREQ_CACHE_FLUSH;
	    ii = new inquiry_info[maxResponse * sizeof(inquiry_info)];
	    
	    numberOfResponses = hci_inquiry(ownId, duration, maxResponse, nullptr, &ii, flags);
	    if( numberOfResponses < 0 )
	    {
	    	delete ii;
	    	throw SocketException("Error scanning for bluetooth devices");
	    }

	    for (int i = 0; i < numberOfResponses; i++) 
	    {
	        ba2str( &(ii + i)->bdaddr, deviceAddress);
			memset(&deviceName, 0, sizeof(deviceName));
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

#ifdef _WIN32

	std::string Socket::getLocalMACAddress()
	{
		// Up to 20 Interfaces
		IP_ADAPTER_INFO AdapterInfo[20];
		DWORD dwBufLen = sizeof(AdapterInfo);

		DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;

		while(std::string(pAdapterInfo->Description).find("Bluetooth") == std::string::npos)
		{
			pAdapterInfo = pAdapterInfo->Next;
		}

		std::stringstream ss;

		for (int i = 0; i < 6; i++)
		{
			ss << std::hex << std::setfill('0');
			ss << std::setw(2) << static_cast<unsigned>(pAdapterInfo->Address[i]);

			if (i != 5)
			{
				ss << ":";
			}
		}

		return ss.str();
	}

#elif __linux__

	std::string Socket::getLocalMACAddress()
	{
		int id;
		bdaddr_t btaddr;
		char localMACAddress[18];
	
		// Get id of local device
		if ((id = hci_get_route(nullptr)) < 0)
		{
			return "";
		}
	
		// Get local bluetooth address
		if (hci_devba(id, &btaddr) < 0)
		{
			return "";
		}
	
		// Convert address to string
		if (ba2str(&btaddr, localMACAddress) < 0)
		{
			return "";
		}
		
		return std::string(localMACAddress);
	}

#endif

} // End namespace kt
