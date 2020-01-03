
#include "ServerSocket.h"
#include "../Socket/Socket.h"
#include "../SocketExceptions/SocketException.hpp"
#include "../SocketExceptions/BindingException.hpp"
#include "../Enums/SocketType.cpp"

#include <iostream>
#include <random>
#include <ctime>
#include <cerrno>
#include <cstring>

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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <unistd.h>

#endif

namespace kt
{
    ServerSocket::ServerSocket(const kt::SocketType type, const unsigned int& port)
    {
        this->port = port;
        this->type = type;

 #ifdef _WIN32

        this->socketDescriptor = INVALID_SOCKET;

        WSADATA wsaData;
        int res = WSAStartup(MAKEWORD(2,2), &wsaData);
        if(res != 0)
        {
            throw SocketException("WSAStartup Failed: " + std::to_string(res));
        }

#endif

        // Randomly allocate port and construct socket
        if (this->port == 0)
        {
            this->randomlyAllocatePort();
        }
        else
        {
            this->constructSocket();
        }
    }

    void ServerSocket::randomlyAllocatePort()
    {
        bool done = false;
        std::random_device rd;
        std::mt19937 gen(rd());
        // Random port number inside the 'dynamic' port range (49152 - 65535)
        std::uniform_int_distribution<> wifiRand(49152, 65535);
        // Random bluetooth ports from 1-30
        std::uniform_int_distribution<> btRand(1, 30);

        while (!done)
        {
            try
            {
                if (this->type == kt::SocketType::Wifi)
                { 
                    this->port = wifiRand(gen);     
                }
                else
                {
                    this->port = btRand(gen);
                }
                this->constructSocket();
                done = true;
            }
            catch(BindingException be)
            {
                // Nothing to do
            }
        }
    }

    ServerSocket::ServerSocket(const ServerSocket& socket)
    {
        this->port = socket.port;
        this->type = socket.type;
        this->socketDescriptor = socket.socketDescriptor;
        this->serverAddress = socket.serverAddress;

#ifdef __linux__

        this->socketSize = socket.socketSize;

#endif
    }

    ServerSocket& ServerSocket::operator=(const ServerSocket& socket)
    {
        this->port = socket.port;
        this->type = socket.type;
        this->socketDescriptor = socket.socketDescriptor;
        this->serverAddress = socket.serverAddress;

#ifdef __linux__

        this->socketSize = socket.socketSize;

#endif

        return *this;
    }

    void ServerSocket::constructSocket()
    {
        if (this->type == kt::SocketType::Wifi)
        {
            this->constructWifiSocket();
        }
        else
        {
            this->constructBluetoothSocket();
            this->setDiscoverable();
        }
    }

#ifdef _WIN32

    void ServerSocket::constructBluetoothSocket()
    {
        SOCKADDR_BTH bluetoothAddress;

        this->socketDescriptor = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

        if (this->socketDescriptor == INVALID_SOCKET)
        {
            throw SocketException("Error establishing BT server socket: " + std::string(std::strerror(errno)));
        }

        this->bluetoothAddress.addressFamily = AF_BTH;
        this->bluetoothAddress.btAddr = 0;
        this->bluetoothAddress.port = this->port;

        if (bind(this->socketDescriptor, (struct sockaddr *) &this->bluetoothAddress, sizeof(SOCKADDR_BTH) ) == SOCKET_ERROR) 
        {
            throw BindingException("Error binding BT connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)) + ". WSA Error: " + std::to_string(WSAGetLastError()));
        }

        if (listen(this->socketDescriptor, 1) == SOCKET_ERROR) 
        {
            this->close();
            throw SocketException("Error Listening on port: " + std::to_string(this->port) + ": " + std::string(std::strerror(errno)));
        }
        // Make discoverable

        /*LPCSADDR_INFO lpCSAddrInfo = nullptr;
        lpCSAddrInfo[0].LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
        lpCSAddrInfo[0].LocalAddr.lpSockaddr = (LPSOCKADDR) &bluetoothAddress;
        lpCSAddrInfo[0].RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
        lpCSAddrInfo[0].RemoteAddr.lpSockaddr = (LPSOCKADDR) &bluetoothAddress;
        lpCSAddrInfo[0].iSocketType = SOCK_STREAM;
        lpCSAddrInfo[0].iProtocol = BTHPROTO_RFCOMM;*/

        /*WSAQUERYSET wsaQuerySet;
        memset(&wsaQuerySet, 0, sizeof(WSAQUERYSET));
        wsaQuerySet.dwSize = sizeof(WSAQUERYSET);
        wsaQuerySet.dwNameSpace = NS_BTH;
        wsaQuerySet.lpServiceClassId = nullptr;

        std::cout << "HERE..." << std::endl;

        if (WSASetService(&wsaQuerySet, RNRSERVICE_REGISTER, 0) == SOCKET_ERROR) 
        {
        	std::cout << "RIP..." << std::endl;
            throw SocketException("Unable to make bluetooth server discoverable: " + std::to_string(WSAGetLastError()));
        }
        std::cout << "DONE!" << std::endl;*/
    }

#elif __linux__

    void ServerSocket::constructBluetoothSocket()
    {
        struct sockaddr_rc localAddress = {0};
        bdaddr_t tmp = ((bdaddr_t) {{0, 0, 0, 0, 0, 0}});
        this->socketSize = sizeof(localAddress);

        this->socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

        if (this->socketDescriptor == -1)
        {
            throw SocketException("Error establishing BT server socket: " + std::string(std::strerror(errno)));
        }

        localAddress.rc_family = AF_BLUETOOTH;
        localAddress.rc_bdaddr = tmp;
        localAddress.rc_channel = static_cast<uint8_t>(port);
        
        if (bind(this->socketDescriptor, (struct sockaddr *)&localAddress, sizeof(localAddress)) == -1)
        {
            throw BindingException("Error binding BT connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)));
        }

        if (listen(this->socketDescriptor, 1) == -1)
        {
            this->close();
            throw SocketException("Error Listening on port " + std::to_string(this->port) + ": " + std::string(std::strerror(errno)));
        }

        // Make discoverable
    }

#endif

#ifdef _WIN32

    void ServerSocket::constructWifiSocket()
    {
        memset(&this->hints, 0, sizeof(this->hints));
        this->hints.ai_family = AF_INET;
		this->hints.ai_socktype = SOCK_STREAM;
		this->hints.ai_protocol = IPPROTO_TCP;
        this->hints.ai_flags = AI_PASSIVE;

        if (getaddrinfo(nullptr, std::to_string(this->port).c_str(), &this->hints, &this->serverAddress) != 0) 
        {
            throw SocketException("Unable to retrieving Wifi serversocket host address. (Self)");
        }

        this->socketDescriptor = socket(this->serverAddress->ai_family, this->serverAddress->ai_socktype, this->serverAddress->ai_protocol);
        if (this->socketDescriptor == INVALID_SOCKET) 
        {
             throw SocketException("Error establishing wifi server socket: " + std::string(std::strerror(errno)));
        }

        if (bind(this->socketDescriptor, this->serverAddress->ai_addr, (int)this->serverAddress->ai_addrlen) == SOCKET_ERROR) 
        {
            throw BindingException("Error binding connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)));
        }

        if(listen(this->socketDescriptor, SOMAXCONN) == SOCKET_ERROR)
        {
            this->close();
            throw SocketException("Error Listening on port " + std::to_string(this->port) + ": " + std::string(std::strerror(errno)));
        }
    }

#elif __linux__

    void ServerSocket::constructWifiSocket()
    {
        this->socketSize = sizeof(serverAddress);
        this->socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

        if (this->socketDescriptor == -1) 
        {
            throw SocketException("Error establishing wifi server socket: " + std::string(std::strerror(errno)));
        }

        this->serverAddress.sin_family = AF_INET;
        this->serverAddress.sin_addr.s_addr = htons(INADDR_ANY);
        this->serverAddress.sin_port = htons(this->port);

        if (bind(this->socketDescriptor, (struct sockaddr*) &this->serverAddress, sizeof(this->serverAddress)) == -1) 
        {
            throw BindingException("Error binding connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)));
        }

        if(listen(this->socketDescriptor, 1) == -1)
        {
            this->close();
            throw SocketException("Error Listening on port " + std::to_string(this->port) + ": " + std::string(std::strerror(errno)));
        }
    }

#endif

#ifdef _WIN32

    void ServerSocket::setDiscoverable()
    {
        throw SocketException("Not yet implemented on Windows.");
        std::cout << "SETTING DISCOVERABLE" << std::endl;
        CSADDR_INFO address[1];

        memset(&address, 0, sizeof(address));
        address[0].LocalAddr.iSockaddrLength = sizeof(socketDescriptor);
        address[0].LocalAddr.lpSockaddr = (struct sockaddr*) &socketDescriptor;
        address[0].iSocketType = SOCK_STREAM;
        address[0].iProtocol = BTHPROTO_RFCOMM;

        WSAQUERYSET wsaQuery;
        memset(&wsaQuery, 0, sizeof(WSAQUERYSET));
        wsaQuery.dwSize = sizeof(WSAQUERYSET);
        wsaQuery.dwNameSpace = NS_BTH;
        wsaQuery.dwNumberOfCsAddrs = 1;
        wsaQuery.lpcsaBuffer = address;

        if (WSASetService(&wsaQuery, RNRSERVICE_REGISTER, 0) != 0)
        {
            throw SocketException("Failed to make device discoverable. " + std::to_string(WSAGetLastError()));
        }
        std::cout << "SETTING DISCOVERABLE" << std::endl;
    }

#elif __linux__

    void ServerSocket::setDiscoverable()
    {
        throw SocketException("Still in progress.");

        hci_dev_req req;
        req.dev_id = 0;
        // req.dev_id = hci_get_route(nullptr);
        req.dev_opt = SCAN_PAGE | SCAN_INQUIRY;

        if (ioctl(this->socketDescriptor, HCISETSCAN, (unsigned long)&req) < 0)
        {
            throw SocketException("Failed to make device discoverable.");            
        }
    }

#endif

    Socket ServerSocket::accept()
    {
#ifdef _WIN32

        SOCKET temp = ::accept(this->socketDescriptor, nullptr, nullptr);

#elif __linux__

        struct sockaddr_rc remoteDevice = {0};
        int temp = ::accept(this->socketDescriptor, (struct sockaddr *) &remoteDevice, &this->socketSize);

        if (this->type == kt::SocketType::Bluetooth)
        {
        	char remoteAddress[1024] = {0};
	        ba2str(&remoteDevice.rc_bdaddr, remoteAddress);

	        std::cout << "Accepted connection from " << remoteAddress << std::endl;
        }

#endif 

        struct sockaddr_in address;
		socklen_t addr_size = sizeof(struct sockaddr_in);
		int res = getpeername(temp, (struct sockaddr *)&address, &addr_size);

        std::string hostname = "";
        unsigned int portnum = 0;
		if (res == 0)
		{ 
			char ip[20];
    		strcpy(ip, inet_ntoa(address.sin_addr));
			hostname = std::string(ip);
            portnum = htons(address.sin_port);
		}

        return Socket(temp, this->type, kt::SocketProtocol::TCP, hostname, portnum);
    }

    unsigned int ServerSocket::getPort() const
    {
        return this->port;
    }

    void ServerSocket::close()
    {
        #ifdef _WIN32

        freeaddrinfo(this->serverAddress);
        closesocket(this->socketDescriptor);

        #elif __linux__

        ::close(this->socketDescriptor);

        #endif
    }

} // End namespace kt
