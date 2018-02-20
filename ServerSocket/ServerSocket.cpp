
#include "ServerSocket.h"
#include "../Socket/Socket.h"

#include <iostream>
#include <cstdlib>
#include <ctime>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

ServerSocket::ServerSocket(const bool isWifi)
{
    this->isWifi = isWifi;

    std::srand(std::time(nullptr));

    do 
    {
        this->port = std::rand() % 65535;
    } while ( !this->constructSocket());
}

ServerSocket::ServerSocket(const int port, const bool isWifi)
{
	this->port = port;
    this->isWifi = isWifi;

    this->constructSocket();
}

bool ServerSocket::constructSocket()
{
    if (this->isWifi)
    {
        return this->constructWifiSocket();
    }
    else
    {
        return this->constructBluetoothSocket();
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

bool ServerSocket::constructBluetoothSocket()
{
    struct sockaddr_rc localAddress = {0};
    bdaddr_t tmp = { };

    socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    if (socketDescriptor < 0) 
    {
        std::cerr << "Error establishing BT server socket..." << std::endl;
        return false;
    }

    localAddress.rc_family = AF_BLUETOOTH;
    localAddress.rc_bdaddr = tmp;
    localAddress.rc_channel = (uint8_t) port;
    if (bind(socketDescriptor, (struct sockaddr *)&localAddress, sizeof(localAddress)) < 0)
    {
        std::cerr << "Error binding connection, the port is already being used..." << std::endl;
        return false;
    }

    if (listen(socketDescriptor, 1) != 0)
    {
        std::cerr << "Error Listening on socket " << socketDescriptor << std::endl;
        close(socketDescriptor);
        return false;
    }

    return true;
}

bool ServerSocket::constructWifiSocket()
{
    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (socketDescriptor < 0) 
    {
        std::cerr << "Error establishing wifi server socket..." << std::endl;
        return false;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htons(INADDR_ANY);
    serverAddress.sin_port = htons(this->port);

    if ( bind(socketDescriptor, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) 
    {
        std::cerr << "Error binding connection, the port is already being used..." << std::endl;
        return false;
    }

    socketSize = sizeof(serverAddress);

    if(listen(socketDescriptor, 1) != 0)
    {
        std::cerr << "Error Listening on socket " << socketDescriptor << std::endl;
        close(socketDescriptor);
        return false;
    }

    return true;
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
