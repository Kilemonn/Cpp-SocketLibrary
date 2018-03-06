
#include "ServerSocket.h"
#include "../Socket/Socket.h"
#include "../SocketExceptions/SocketError.hpp"
#include "../SocketExceptions/BindingError.hpp"

#include <iostream>
#include <random>
#include <ctime>

#ifdef _WIN32

#include <windows.h>

#elif __linux__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#endif

// Throws SocketError when instance cannot bind or listen
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
        std::uniform_int_distribution<> btRand(1, 10);

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
            catch(BindingError be)
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

ServerSocket::ServerSocket(const ServerSocket& socket)
{
    this->port = socket.port;
    this->isWifi = socket.isWifi;
    this->serverAddress = socket.serverAddress;
    this->socketSize = socket.socketSize;
}

ServerSocket& ServerSocket::operator=(const ServerSocket& socket)
{
    this->close();

    this->port = socket.port;
    this->isWifi = socket.isWifi;
    this->serverAddress = socket.serverAddress;
    this->socketSize = socket.socketSize;

    return *this;
}

ServerSocket::~ServerSocket()
{
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

void ServerSocket::constructBluetoothSocket()
{
    struct sockaddr_rc localAddress = {0};
    bdaddr_t tmp = ((bdaddr_t) {{0, 0, 0, 0, 0, 0}});
    this->socketSize = sizeof(localAddress);

    socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    if (socketDescriptor == -1) 
    {
        throw SocketError("Error establishing BT server socket...");
    }

    localAddress.rc_family = AF_BLUETOOTH;
    localAddress.rc_bdaddr = tmp;
    localAddress.rc_channel = static_cast<uint8_t>(port);
    
    if (bind(socketDescriptor, (struct sockaddr *)&localAddress, sizeof(localAddress)) == -1)
    {
        throw BindingError("Error binding connection, the port " + std::to_string(this->port) + " is already being used...");
    }

    if (listen(socketDescriptor, 1) == -1)
    {
        this->close();
        throw SocketError("Error Listening on port " + std::to_string(this->port));
    }
}

void ServerSocket::constructWifiSocket()
{
    this->socketSize = sizeof(serverAddress);
    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (socketDescriptor == -1) 
    {
        throw SocketError("Error establishing wifi server socket...");
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htons(INADDR_ANY);
    serverAddress.sin_port = htons(this->port);

    if ( bind(socketDescriptor, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) == -1) 
    {
        throw BindingError("Error binding connection, the port " + std::to_string(this->port) + " is already being used...");
    }

    if(listen(socketDescriptor, 1) == -1)
    {
        this->close();
        throw SocketError("Error Listening on port " + std::to_string(this->port));
    }
}

Socket ServerSocket::accept()
{
    int temp = ::accept(socketDescriptor, (struct sockaddr *) &serverAddress, &socketSize);

    return Socket(temp, isWifi);
}

unsigned int ServerSocket::getPort() const
{
    return this->port;
}

void ServerSocket::close()
{
    ::close(socketDescriptor);
}
