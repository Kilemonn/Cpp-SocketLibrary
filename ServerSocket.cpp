
#include "ServerSocket.h"
#include "Socket.h"

#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

ServerSocket::ServerSocket(const int port)
{
	this->port = port;

	this->doConnect();
}

void ServerSocket::doConnect()
{
    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (socketDescriptor < 0) 
    {
        std::cerr << "\nError establishing socket..." << std::endl;
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(this->port);

    if ( bind(socketDescriptor, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) 
    {
        std::cerr << "Error binding connection, the socket is already being used..." << std::endl;
        return;
    }

    size = sizeof(server_addr);

    if(listen(socketDescriptor, 1) != 0)
    {
        std::cerr << "Error Listening on socket " << socketDescriptor << std::endl;
        close(socketDescriptor);
        return;
    }
}

Socket ServerSocket::acceptConnection()
{
	int temp = accept(socketDescriptor, (struct sockaddr *) &server_addr, &size);

	return Socket(temp);
}
