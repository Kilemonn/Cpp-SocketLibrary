
#include "ServerSocket.h"
#include "../socket/Socket.h"
#include "../socketexceptions/SocketException.hpp"
#include "../socketexceptions/BindingException.hpp"
#include "../socketexceptions/TimeoutException.hpp"
#include "../enums/SocketProtocol.cpp"
#include "../enums/SocketType.cpp"

#include <iostream>
#include <random>
#include <ctime>
#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <unistd.h>

namespace kt
{
    /**
     * ServerSocket constructor. Creates a wifi/bluetooth ServerSocket and begins listening for connections.
     *
     * @param type - Determines whether this ServerSocket is a wifi or bluetooth ServerSocket.
     * @param port - The port number for this server to communicate through. If value is not passed in a random, available port number will be assigned.
     * @param connectionBacklogSize - You can enter a value here to specify the length of the server connection pool. The default value is 20.
     * 
     * @throw SocketException - If the ServerSocket is unable to be instantiated or begin listening.
     * @throw BindingException - If the ServerSocket is unable to bind to the specific port specified.
     */
    ServerSocket::ServerSocket(const kt::SocketType type, const unsigned int& port, const unsigned int& connectionBacklogSize)
    {
        this->port = port;
        this->type = type;

        if (this->type == kt::SocketType::None)
        {
            throw SocketException("Failed to create ServerSocket with 'None' SocketType.");
        }

        // Randomly allocate port and construct socket
        if (this->port == 0)
        {
            this->randomlyAllocatePort(connectionBacklogSize);
        }
        else
        {
            this->constructSocket(connectionBacklogSize);
        }
    }

    /**
     * ServerSocket copy constructor.
     * 
     * @param socket - The ServerSocket object to be copied.
     */
    ServerSocket::ServerSocket(const ServerSocket& socket)
    {
        this->port = socket.port;
        this->type = socket.type;
        this->socketDescriptor = socket.socketDescriptor;
        this->serverAddress = socket.serverAddress;
        this->socketSize = socket.socketSize;
    }

    /**
     * Overloaded assignment operator for the ServerSocket class.
     * 
     * @param socket - The ServerSocket object to be copied.
     * 
     * @return the copied socket
     */
    ServerSocket& ServerSocket::operator=(const ServerSocket& socket)
    {
        this->port = socket.port;
        this->type = socket.type;
        this->socketDescriptor = socket.socketDescriptor;
        this->serverAddress = socket.serverAddress;
        this->socketSize = socket.socketSize;

        return *this;
    }

    void ServerSocket::constructSocket(const unsigned int& connectionBacklogSize)
    {
        if (this->type == kt::SocketType::Wifi)
        {
            this->constructWifiSocket(connectionBacklogSize);
        }
        else if (this->type == kt::SocketType::Bluetooth)
        {
            this->constructBluetoothSocket(connectionBacklogSize);
            this->setDiscoverable();
        }
    }

    void ServerSocket::constructBluetoothSocket(const unsigned int& connectionBacklogSize)
    {
        struct sockaddr_rc localAddress = {0};
        this->socketSize = sizeof(localAddress);

        this->socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

        if (this->socketDescriptor == -1)
        {
            throw SocketException("Error establishing BT server socket: " + std::string(std::strerror(errno)));
        }

        localAddress.rc_family = AF_BLUETOOTH;
        localAddress.rc_bdaddr = ((bdaddr_t) {{0, 0, 0, 0, 0, 0}});
        localAddress.rc_channel = static_cast<uint8_t>(port);
        
        if (bind(this->socketDescriptor, (struct sockaddr *)&localAddress, sizeof(localAddress)) == -1)
        {
            throw BindingException("Error binding BT connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)));
        }

        if (listen(this->socketDescriptor, connectionBacklogSize) == -1)
        {
            this->close();
            throw SocketException("Error Listening on port " + std::to_string(this->port) + ": " + std::string(std::strerror(errno)));
        }

        // Make discoverable
    }

    void ServerSocket::constructWifiSocket(const unsigned int& connectionBacklogSize)
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

        if(listen(this->socketDescriptor, connectionBacklogSize) == -1)
        {
            this->close();
            throw SocketException("Error Listening on port " + std::to_string(this->port) + ": " + std::string(std::strerror(errno)));
        }
    }

    /**
     * 
     * 
     * 
     */
    void ServerSocket::randomlyAllocatePort(const unsigned int &connectionBacklogSize)
    {
        const unsigned short maxAttempts = 50;
        std::random_device rd;
        // Random wifi port range inside the 'dynamic' port range (49152 - 65535)
        const unsigned int WIFI_LOWER_BOUND = 49152;
        const unsigned int WIFI_UPPER_BOUND = 65535;
        // Random bluetooth ports from 1-30
        const unsigned int BLUETOOTH_LOWER_BOUND = 1;
        const unsigned int BLUETOOTH_UPPER_BOUND = 30;
        
        unsigned int upperBound = 0;
        unsigned int lowerBound = 0;

        if (this->type == kt::SocketType::Wifi)
        {
            lowerBound = WIFI_LOWER_BOUND;
            upperBound = WIFI_UPPER_BOUND;
        }
        else if (this->type == kt::SocketType::Bluetooth)
        {
            lowerBound = BLUETOOTH_LOWER_BOUND;
            upperBound = BLUETOOTH_UPPER_BOUND;
        }
        else
        {
            throw BindingException("Unable to allocate port since SocketType is None.");
        }
        
        std::uniform_int_distribution<unsigned int> randDist(lowerBound, upperBound);

        // Only try to allocate the port 50 times, if it still fails then throw
        for (unsigned int i = 0; i < maxAttempts; i++)
        {
            try
            {
                this->port = randDist(rd);
                this->constructSocket(connectionBacklogSize);
                return;
            }
            catch (BindingException& be)
            {
                // Nothing to do
            }
        }

        throw BindingException("Failed to randomly allocate port for ServerSocket after " + std::to_string(maxAttempts) + " attempts.");
    }

    void ServerSocket::setDiscoverable()
    {
        throw SocketException("ServerSocket::setDiscoverable() not implemented.");

        hci_dev_req req;
        req.dev_id = 0;
        // req.dev_id = hci_get_route(nullptr);
        req.dev_opt = SCAN_PAGE | SCAN_INQUIRY;

        if (ioctl(this->socketDescriptor, HCISETSCAN, (unsigned long)&req) < 0)
        {
            throw SocketException("Failed to make device discoverable.");            
        }
    }

    /**
     * @return the *kt::SocketType* for this *kt::Socket*.
     */
    kt::SocketType ServerSocket::getType() const
    {
        return this->type;
    }

    /**
     * Used to get the port number that the ServerSocket is listening on.
     * @return An unsigned int of the port number that the ServerSocket is listening on.
     */
    unsigned int ServerSocket::getPort() const
    {
        return this->port;
    }

    /**
     * Used to accept a connection on the specific port. 
     * Upon accepting a new connection it will return a Socket object used to communicate with the receiver.
     * 
     * @param timeout - indicates how long the socket should be polled for before assuming there is no response. Default is 0 (unlimited).
     * 
     * @returns kt::Socket object of the receiver who has just connected to the kt::ServerSocket.
     */
    Socket ServerSocket::accept(const unsigned int& timeout)
    {
        if (timeout != 0)
        {
            fd_set sready;
            struct timeval timeOutVal;
            memset((char *)&timeOutVal, 0, sizeof(timeOutVal));
            timeOutVal.tv_usec = static_cast<int>(timeout);

            FD_ZERO(&sready);
            FD_SET((unsigned int)this->socketDescriptor, &sready);

            int res = select(this->socketDescriptor + 1, &sready, nullptr, nullptr, &timeOutVal);

            if (res == -1)
            {
                throw kt::SocketException("Failed to poll as socket is no longer valid.");
            }
            else if (res == 0)
            {
                throw kt::TimeoutException("No applicable connections could be accepted during the time period specified " + std::to_string(timeout) + " microseconds.");
            }
        }

        struct sockaddr_rc remoteDevice = {0};
        int temp = ::accept(this->socketDescriptor, (struct sockaddr *) &remoteDevice, &this->socketSize);

        if (temp == -1)
        {
            throw SocketException("Failed to accept connection. Socket is in an invalid state.");
        }

        if (this->type == kt::SocketType::Bluetooth)
        {
        	char remoteAddress[1024] = {0};
	        ba2str(&remoteDevice.rc_bdaddr, remoteAddress);

	        std::cout << "Accepted connection from " << remoteAddress << std::endl;
        }

        struct sockaddr_in address;
		socklen_t addr_size = sizeof(struct sockaddr_in);
		int res = getpeername(temp, (struct sockaddr *)&address, &addr_size);

        std::string hostname;
        unsigned int portNum = 0;
		if (res == 0)
		{ 
			char ip[20];
    		strcpy(ip, inet_ntoa(address.sin_addr));
			hostname = std::string(ip);
            portNum = htons(address.sin_port);
		}

        return Socket(temp, this->type, kt::SocketProtocol::TCP, hostname, portNum);
    }

    /**
     * Closes the existing connection. If no connection is open, then it will do nothing.
     * 
     * NOTE: This should be called before the server goes out of scope
     */
    void ServerSocket::close() const
    {
        ::close(this->socketDescriptor);
    }

} // End namespace kt
