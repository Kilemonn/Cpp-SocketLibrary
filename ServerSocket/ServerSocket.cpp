
#include "ServerSocket.h"
#include "../Socket/Socket.h"
#include "../SocketError/SocketError.hpp"

#include <iostream>
#include <cstdlib>
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
ServerSocket::ServerSocket(const bool isWifi, const int& port)
{
	this->port = port;
    this->isWifi = isWifi;
    bool done = false;

    // Randomly allocate port
    if (this->port == 0)
    {
        std::srand(std::time(nullptr));

        while (!done)
        {
            try
            {
                this->port = std::rand() % 65535;
                this->constructSocket();
                done = true;
            }
            catch(SocketError serr)
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
    this->port = socket.port;
    this->isWifi = socket.isWifi;
    this->serverAddress = socket.serverAddress;
    this->socketSize = socket.socketSize;

    return *this;
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
    bdaddr_t tmp = { };

    socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    if (socketDescriptor < 0) 
    {
        throw SocketError("Error establishing BT server socket...");
    }

    localAddress.rc_family = AF_BLUETOOTH;
    localAddress.rc_bdaddr = tmp;
    localAddress.rc_channel = (uint8_t) port;
    if (bind(socketDescriptor, (struct sockaddr *)&localAddress, sizeof(localAddress)) != 0)
    {
        throw SocketError("Error binding connection, the port is already being used...");
    }

    if (listen(socketDescriptor, 1) != 0)
    {
        close(socketDescriptor);
        throw SocketError("Error Listening on port " + std::to_string(this->port));
    }
}

void ServerSocket::constructWifiSocket()
{
    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (socketDescriptor == -1) 
    {
        throw SocketError("Error establishing wifi server socket...");
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htons(INADDR_ANY);
    serverAddress.sin_port = htons(this->port);

    if ( bind(socketDescriptor, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) != 0) 
    {
        throw SocketError("Error binding connection, the port is already being used...");
    }

    socketSize = sizeof(serverAddress);

    if(listen(socketDescriptor, 1) != 0)
    {
        close(socketDescriptor);
        throw SocketError("Error Listening on port " + std::to_string(this->port));
    }
}

Socket ServerSocket::acceptConnection()
{
    int temp = accept(socketDescriptor, (struct sockaddr *) &serverAddress, &socketSize);

    return Socket(temp, isWifi);
}

int ServerSocket::getPort() const
{
    return this->port;
}
