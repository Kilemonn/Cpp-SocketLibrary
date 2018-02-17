
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

	socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (socketDescriptor < 0) 
    {
        std::cerr << "\nError establishing socket..." << std::endl;
        return;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htons(INADDR_ANY);
    serverAddress.sin_port = htons(this->port);

    if ( bind(socketDescriptor, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) 
    {
        std::cerr << "Error binding connection, the socket is already being used..." << std::endl;
        return;
    }

    socketSize = sizeof(serverAddress);

    if(listen(socketDescriptor, 1) != 0)
    {
        std::cerr << "Error Listening on socket " << socketDescriptor << std::endl;
        close(socketDescriptor);
        return;
    }
}

Socket ServerSocket::acceptConnection()
{
	int temp = accept(socketDescriptor, (struct sockaddr *) &serverAddress, &socketSize);

	return Socket(temp);
}
