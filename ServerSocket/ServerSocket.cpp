
#include "ServerSocket.h"
#include "../Socket/Socket.h"
#include "../SocketExceptions/SocketException.hpp"
#include "../SocketExceptions/BindingException.hpp"

#include <iostream>
#include <random>
#include <ctime>

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
#include <unistd.h>

#endif

namespace kt
{
    #ifdef _WIN32

    ServerSocket::ServerSocket(const bool isWifi, const unsigned int& port)
    {
        socketDescriptor = INVALID_SOCKET;
        this->port = port;
        this->isWifi = isWifi;
        bool done = false;

        WSADATA wsaData;
        int res = WSAStartup(MAKEWORD(2,2), &wsaData);
        if(res != 0)
        {
            throw SocketException("WSAStartup Failed: " + std::to_string(res));
        }

        // Randomly allocate port
        if (this->port == 0)
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            // Random port number inside the 'dynamic' port range (49152 - 65535)
            std::uniform_int_distribution<> wifiRand(49152, 65535);
            // Random bluetooth ports from 1-10
            std::uniform_int_distribution<> btRand(1, 30);

            while (!done)
            {
                try
                {
                    if (isWifi)
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
        else
        {
            this->constructSocket();
        }

    }

    #elif __linux__

    // Throws SocketException when instance cannot bind or listen
    ServerSocket::ServerSocket(const bool isWifi, const unsigned int& port)
    {
    	this->port = port;
        this->isWifi = isWifi;
        bool done = false;

        // Randomly allocate port
        if (this->port == 0)
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            // Random port number inside the 'dynamic' port range (49152 - 65535)
            std::uniform_int_distribution<> wifiRand(49152, 65535);
            // Random bluetooth ports from 1-10
            std::uniform_int_distribution<> btRand(1, 30);

            while (!done)
            {
                try
                {
                    if (isWifi)
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
        else
        {
            this->constructSocket();
        }
    }

    #endif


    #ifdef _WIN32

    ServerSocket::ServerSocket(const ServerSocket& socket)
    {
        this->port = socket.port;
        this->isWifi = socket.isWifi;
        this->socketDescriptor = socket.socketDescriptor;
        this->serverAddress = socket.serverAddress;
    }

    #elif __linux__

    ServerSocket::ServerSocket(const ServerSocket& socket)
    {
        this->port = socket.port;
        this->isWifi = socket.isWifi;
        this->socketDescriptor = socket.socketDescriptor;
        this->serverAddress = socket.serverAddress;
        this->socketSize = socket.socketSize;
    }

    #endif

    #ifdef _WIN32

    ServerSocket& ServerSocket::operator=(const ServerSocket& socket)
    {
        this->close();

        this->port = socket.port;
        this->isWifi = socket.isWifi;
        this->socketDescriptor = socket.socketDescriptor;
        this->serverAddress = socket.serverAddress;

        return *this;
    }

    #elif __linux__

    ServerSocket& ServerSocket::operator=(const ServerSocket& socket)
    {
        this->close();

        this->port = socket.port;
        this->isWifi = socket.isWifi;
        this->serverAddress = socket.serverAddress;
        this->socketSize = socket.socketSize;

        return *this;
    }

    #endif

    ServerSocket::~ServerSocket()
    {
        #ifdef _WIN32

        // WSACleanup();
        freeaddrinfo(serverAddress);

        #endif

        this->close();
    }

    void ServerSocket::constructSocket()
    {
        if (this->isWifi)
        {
            this->constructWifiSocket();
        }
        else
        {
            this->constructBluetoothSocket();
        }
    }


    #ifdef _WIN32

    void ServerSocket::constructBluetoothSocket()
    {
        // throw SocketException("Bluetooth servers are not supported in Windows.");

        SOCKADDR_BTH bluetoothAddress;

        socketDescriptor = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

        if (socketDescriptor == INVALID_SOCKET)
        {
            throw SocketException("Error establishing BT server socket...");
        }

        bluetoothAddress.addressFamily = AF_BTH;
        bluetoothAddress.btAddr = 0;
        bluetoothAddress.port = this->port;

        if (bind(socketDescriptor, (struct sockaddr *) &bluetoothAddress, sizeof(SOCKADDR_BTH) ) == SOCKET_ERROR) 
        {
            throw BindingException("Error binding BT connection, the port " + std::to_string(this->port) 
                + " is already being used. WSA Error: " + std::to_string(WSAGetLastError()));
        }

        if (listen(socketDescriptor, 1) == SOCKET_ERROR) 
        {
            this->close();
            throw SocketException("Error Listening on port " + std::to_string(this->port));
        }
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
            throw SocketException("Error establishing BT server socket...");
        }

        localAddress.rc_family = AF_BLUETOOTH;
        localAddress.rc_bdaddr = tmp;
        localAddress.rc_channel = static_cast<uint8_t>(port);
        
        if (bind(socketDescriptor, (struct sockaddr *)&localAddress, sizeof(localAddress)) == -1)
        {
            throw BindingException("Error binding BT connection, the port " + std::to_string(this->port) + " is already being used...");
        }

        if (listen(socketDescriptor, 1) == -1)
        {
            this->close();
            throw SocketException("Error Listening on port " + std::to_string(this->port));
        }
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

        res = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &serverAddress);

        if (res != 0) 
        {
            throw SocketException("Unable to retrieving Wifi serversocket host address. (Self)");
        }

        socketDescriptor = socket(serverAddress->ai_family, serverAddress->ai_socktype, serverAddress->ai_protocol);
        if (socketDescriptor == INVALID_SOCKET) 
        {
             throw SocketException("Error establishing wifi server socket...");
        }

        res = bind( socketDescriptor, serverAddress->ai_addr, (int)serverAddress->ai_addrlen);
        if (res == SOCKET_ERROR) 
        {
            throw BindingException("Error binding connection, the port " + std::to_string(this->port) + " is already being used...");
        }

        if( listen( socketDescriptor, SOMAXCONN ) == SOCKET_ERROR ) 
        {
            this->close();
            throw SocketException("Error Listening on port " + std::to_string(this->port));
        }
    }

    #elif __linux__

    void ServerSocket::constructWifiSocket()
    {
        this->socketSize = sizeof(serverAddress);
        socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

        if (socketDescriptor == -1) 
        {
            throw SocketException("Error establishing wifi server socket...");
        }

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = htons(INADDR_ANY);
        serverAddress.sin_port = htons(this->port);

        if ( bind(socketDescriptor, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) == -1) 
        {
            throw BindingException("Error binding connection, the port " + std::to_string(this->port) + " is already being used...");
        }

        if(listen(socketDescriptor, 1) == -1)
        {
            this->close();
            throw SocketException("Error Listening on port " + std::to_string(this->port));
        }
    }

    #endif

    Socket ServerSocket::accept()
    {
        #ifdef _WIN32

        SOCKET temp = ::accept(socketDescriptor, NULL, NULL);

        #elif __linux__

        struct sockaddr_rc remoteDevice = {0};
        int temp = ::accept(socketDescriptor, (struct sockaddr *) &remoteDevice, &socketSize);
        char remoteAddress[1024] = {0};
        ba2str(&remoteDevice.rc_bdaddr, remoteAddress);

        std::cout << "Accepted connection from " << remoteAddress << std::endl;

        #endif 

        return Socket(temp, isWifi);
    }

    unsigned int ServerSocket::getPort() const
    {
        return this->port;
    }

    void ServerSocket::close()
    {
        #ifdef _WIN32

        closesocket(socketDescriptor);

        #elif __linux__

        ::close(socketDescriptor);

        #endif
    }

} // End namespace kt
