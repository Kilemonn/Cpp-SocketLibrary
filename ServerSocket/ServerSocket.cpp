
#include "ServerSocket.h"
#include "../Socket/Socket.h"
#include "../SocketExceptions/SocketException.hpp"
#include "../SocketExceptions/BindingException.hpp"
#include "../Enums/SocketProtocol.cpp"
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
    #ifdef _WIN32

    ServerSocket::ServerSocket(const kt::SocketType type, const unsigned int& port)
    {
        socketDescriptor = INVALID_SOCKET;
        this->port = port;
        this->type = type;

        WSADATA wsaData;
        int res = WSAStartup(MAKEWORD(2,2), &wsaData);
        if(res != 0)
        {
            throw SocketException("WSAStartup Failed: " + std::to_string(res));
        }

        // Randomly allocate port and construct socket
        if (this->port == 0)
        {
            randomlyAllocatePort();
        }
        else
        {
            this->constructSocket();
        }

    }

    #elif __linux__

    // Throws SocketException when instance cannot bind or listen
    ServerSocket::ServerSocket(const kt::SocketType type, const unsigned int& port)
    {
    	this->port = port;
        this->type = type;

        // Randomly allocate port and construct socket
        if (this->port == 0)
        {
            randomlyAllocatePort();
        }
        else
        {
            this->constructSocket();
        }
    }

    #endif

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
                if (type == kt::SocketType::Wifi)
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

        socketDescriptor = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

        if (socketDescriptor == INVALID_SOCKET)
        {
            throw SocketException("Error establishing BT server socket: " + std::string(std::strerror(errno)));
        }

        bluetoothAddress.addressFamily = AF_BTH;
        bluetoothAddress.btAddr = 0;
        bluetoothAddress.port = this->port;

        if (bind(socketDescriptor, (struct sockaddr *) &bluetoothAddress, sizeof(SOCKADDR_BTH) ) == SOCKET_ERROR) 
        {
            throw BindingException("Error binding BT connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)) + ". WSA Error: " + std::to_string(WSAGetLastError()));
        }

        if (listen(socketDescriptor, 1) == SOCKET_ERROR) 
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
        ZeroMemory(&wsaQuerySet, sizeof(WSAQUERYSET));
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

        socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

        if (socketDescriptor == -1)
        {
            throw SocketException("Error establishing BT server socket: " + std::string(std::strerror(errno)));
        }

        localAddress.rc_family = AF_BLUETOOTH;
        localAddress.rc_bdaddr = tmp;
        localAddress.rc_channel = static_cast<uint8_t>(port);
        
        if (bind(socketDescriptor, (struct sockaddr *)&localAddress, sizeof(localAddress)) == -1)
        {
            throw BindingException("Error binding BT connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)));
        }

        if (listen(socketDescriptor, 1) == -1)
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
        int res;
        ZeroMemory(&hints, sizeof (hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        res = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &serverAddress);

        if (res != 0) 
        {
            throw SocketException("Unable to retrieving Wifi serversocket host address. (Self)");
        }

        socketDescriptor = socket(serverAddress->ai_family, serverAddress->ai_socktype, serverAddress->ai_protocol);
        if (socketDescriptor == INVALID_SOCKET) 
        {
             throw SocketException("Error establishing wifi server socket: " + std::string(std::strerror(errno)));
        }

        res = bind( socketDescriptor, serverAddress->ai_addr, (int)serverAddress->ai_addrlen);
        if (res == SOCKET_ERROR) 
        {
            throw BindingException("Error binding connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)));
        }

        if( listen( socketDescriptor, SOMAXCONN ) == SOCKET_ERROR ) 
        {
            this->close();
            throw SocketException("Error Listening on port " + std::to_string(this->port) + ": " + std::string(std::strerror(errno)));
        }
    }

    #elif __linux__

    void ServerSocket::constructWifiSocket()
    {
        this->socketSize = sizeof(serverAddress);
        socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

        if (socketDescriptor == -1) 
        {
            throw SocketException("Error establishing wifi server socket: " + std::string(std::strerror(errno)));
        }

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = htons(INADDR_ANY);
        serverAddress.sin_port = htons(this->port);

        if ( bind(socketDescriptor, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) == -1) 
        {
            throw BindingException("Error binding connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)));
        }

        if(listen(socketDescriptor, 1) == -1)
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

        ZeroMemory(&address, sizeof(address));
        address[0].LocalAddr.iSockaddrLength = sizeof(socketDescriptor);
        address[0].LocalAddr.lpSockaddr = (struct sockaddr*) &socketDescriptor;
        address[0].iSocketType = SOCK_STREAM;
        address[0].iProtocol = BTHPROTO_RFCOMM;

        WSAQUERYSET wsaQuery;
        ZeroMemory(&wsaQuery, sizeof(WSAQUERYSET));
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

        if (ioctl(socketDescriptor, HCISETSCAN, (unsigned long)&req) < 0)
        {
            throw SocketException("Failed to make device discoverable.");            
        }
    }

    #endif

    Socket ServerSocket::accept()
    {
        #ifdef _WIN32

        SOCKET temp = ::accept(socketDescriptor, nullptr, nullptr);

        #elif __linux__

        struct sockaddr_rc remoteDevice = {0};
        int temp = ::accept(socketDescriptor, (struct sockaddr *) &remoteDevice, &socketSize);

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

        return Socket(temp, this->type, hostname, portnum);
    }

    unsigned int ServerSocket::getPort() const
    {
        return this->port;
    }

    void ServerSocket::close()
    {
        #ifdef _WIN32

        freeaddrinfo(serverAddress);
        closesocket(socketDescriptor);

        #elif __linux__

        ::close(socketDescriptor);

        #endif
    }

} // End namespace kt
