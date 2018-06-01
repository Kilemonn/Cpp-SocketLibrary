
#include "Socket.h"
#include "../SocketExceptions/SocketException.hpp"

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

#endif

namespace kt
{
	Socket::Socket()
	{
		//Nothing to do
	}

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
	    	throw SocketException("WSAStartup Failed. " + std::to_string(res));
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
	// Throws SocketException when can't connect to server
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

		freeaddrinfo(serverAddress);

		#endif

		this->close();
	}


	#ifdef _WIN32

	void Socket::constructBluetoothSocket()
	{
		// throw SocketException("Bluetooth sockets are not supported in Windows.");

		socketDescriptor = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

		if (socketDescriptor == INVALID_SOCKET)
		{
			throw SocketException("Error establishing Bluetooth socket: " + std::string(std::strerror(errno)));
		}

		bluetoothAddress.addressFamily = AF_BTH;
	    bluetoothAddress.btAddr = std::stoull(this->hostname);
	    bluetoothAddress.port = this->port;

	    if (connect(socketDescriptor, (struct sockaddr *) &bluetoothAddress, sizeof(SOCKADDR_BTH)) == SOCKET_ERROR)
	    {
	    	throw SocketException("Error connecting to Bluetooth server: " + std::string(std::strerror(errno)));
	    }
	}

	#elif __linux__

	void Socket::constructBluetoothSocket()
	{
		socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

		if (socketDescriptor == -1)
	    {
	    	throw SocketException("Error establishing Bluetooth socket: " + std::string(std::strerror(errno)));
	    }

	    bluetoothAddress.rc_family = AF_BLUETOOTH;
	    bluetoothAddress.rc_channel = (uint8_t) port;
	    str2ba(this->hostname.c_str(), &bluetoothAddress.rc_bdaddr);

	   	if (connect(socketDescriptor, (struct sockaddr *) &bluetoothAddress, sizeof(bluetoothAddress)) == -1)
	   	{
	   		throw SocketException("Error connecting to Bluetooth server: " + std::string(std::strerror(errno)));
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
		    throw SocketException("Unable to retrieving host address: " + std::string(std::strerror(errno)));
		}

		socketDescriptor = socket(serverAddress->ai_family, serverAddress->ai_socktype, serverAddress->ai_protocol);
		if (socketDescriptor == INVALID_SOCKET) 
		{
		    throw SocketException("Error establishing Wifi socket: " + std::string(std::strerror(errno)));
		}

	    res = connect(socketDescriptor, serverAddress->ai_addr, (int)serverAddress->ai_addrlen);
		if(res == SOCKET_ERROR)
		{
			throw SocketException("Error connecting to Wifi server: " + std::string(std::strerror(errno)));
		}
	}

	#elif __linux__

	void Socket::constructWifiSocket()
	{
		struct hostent* server = gethostbyname(this->hostname.c_str());
	    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

	    if (socketDescriptor == -1)
	    {
	    	throw SocketException("Error establishing Wifi socket: " + std::string(std::strerror(errno)));
	    }

	    bzero((char *) &serverAddress, sizeof(serverAddress));
	    serverAddress.sin_family = AF_INET;
	    bcopy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr, server->h_length);
	    serverAddress.sin_port = htons(this->port);

		if (connect(socketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
		{
			throw SocketException("Error connecting to Wifi server: " + std::string(std::strerror(errno)));
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

	bool Socket::ready(unsigned long timeout) const
	{
		fd_set sready;
		struct timeval timeOutVal;
		timeOutVal.tv_usec = timeout;

		FD_ZERO(&sready);
		FD_SET((unsigned int)this->socketDescriptor, &sready);
		memset((char*) &timeOutVal, 0, sizeof(timeOutVal));

		int res = select(this->socketDescriptor + 1, &sready, nullptr, nullptr, &timeOutVal);
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

	std::string Socket::receiveAmount(const unsigned int amountToReceive) const
	{
		if (amountToReceive == 0)
		{
			return "";
		}

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
			
			if (flag < 1)
			{
				return std::string(result);
			}
			result += std::string(data);
			counter += flag;
		}
		return std::move(std::string(result));
	}

	// Do not pass in '\0' as a delimiter
	std::string Socket::receiveToDelimiter(const char delimiter) const
	{
		if (delimiter == '\0')
		{
			throw SocketException("The null terminator '\0' is an invalid delimiter.");
		}

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

		return std::move(data.substr(0, (data.size() - 1)));
	}

	std::string Socket::receiveAll() const
	{
		const unsigned long oneHundredMS = 100000; // 1 second in micro seconds
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

		ZeroMemory(&wsaQuery, sizeof(WSAQUERYSET));
		wsaQuery.dwSize = sizeof(WSAQUERYSET);
		wsaQuery.dwNameSpace = NS_BTH;
		wsaQuery.lpcsaBuffer = nullptr;

		int res = WSALookupServiceBegin(&wsaQuery, LUP_CONTAINERS, &hLoopUp);

		if (res == SOCKET_ERROR)
		{
			throw SocketException("Unable to search for devices. Could not begin search.");
		}
		
		ZeroMemory(&pQuerySet, sizeof(WSAQUERYSET));
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

		/*
		throw SocketException("Not implemented yet.");

		struct ifaddrs *ifaddr = nullptr;
	    struct ifaddrs *ifa = nullptr;

	    if (getifaddrs(&ifaddr) != -1)
	    {
			for ( ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
			{
				if ( (ifa->ifa_addr) && (ifa->ifa_addr->sa_family == AF_PACKET) )
				{
					struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;

					if (std::string(ifa->ifa_name).find("hci") != std::string::npos)
					{
						std::stringstream ss;
						
						for (int i = 0; i < s->sll_halen; i++)
						{
							ss << std::hex << std::setfill('0');
							ss << std::setw(2) << static_cast<unsigned>(s->sll_addr[i]);

							if ((i + 1) != s->sll_halen)
							{
								ss << ":";
							}
						}
						freeifaddrs(ifaddr);
						return ss.str();
					}
				}
			}
			freeifaddrs(ifaddr);
	    }
	    return "";
		*/
	}

	#endif

} // End namespace kt
