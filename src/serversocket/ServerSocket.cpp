
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

    /**
     * Default constructor. Should be provided by the compiler, but there has been some scenarios where this is required. 
     */
    ServerSocket::ServerSocket()
    {
        // Nothing to do, defaults set in header file
    }

    /**
     * ServerSocket constructor. Creates a wifi/bluetooth ServerSocket and begins listening for connections.
     *
     * @param type - Determines whether this ServerSocket is a wifi or bluetooth ServerSocket.
     * @param port - The port number for this server to communicate through. If value is not passed in a random, available port number will be assigned.
     * @param connectionBacklogSize - You can enter a value here to specify the length of the server connection pool. The default value is 20.
     * 
     * @throw SocketException - If the ServerSocket is unable to be instanciated or begin listening.
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

 #ifdef _WIN32

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

#ifdef __linux__

        this->socketSize = socket.socketSize;

#endif
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

#ifdef __linux__

        this->socketSize = socket.socketSize;

#endif

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

#ifdef _WIN32

    void ServerSocket::constructBluetoothSocket(const unsigned int& connectionBacklogSize)
    {
        SOCKADDR_BTH bluetoothAddress;

        this->socketDescriptor = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

        if (this->socketDescriptor == 0)
        {
            throw SocketException("Error establishing BT server socket: " + std::string(std::strerror(errno)));
        }

        this->bluetoothAddress.addressFamily = AF_BTH;
        this->bluetoothAddress.btAddr = 0;
        this->bluetoothAddress.port = this->port;

        if (bind(this->socketDescriptor, (struct sockaddr *) &this->bluetoothAddress, sizeof(SOCKADDR_BTH) ) == -1) 
        {
            throw BindingException("Error binding BT connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)) + ". WSA Error: " + std::to_string(WSAGetLastError()));
        }

        if (listen(this->socketDescriptor, connectionBacklogSize) == -1) 
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

        if (WSASetService(&wsaQuerySet, RNRSERVICE_REGISTER, 0) == -1) 
        {
        	std::cout << "RIP..." << std::endl;
            throw SocketException("Unable to make bluetooth server discoverable: " + std::to_string(WSAGetLastError()));
        }
        std::cout << "DONE!" << std::endl;*/
    }

#elif __linux__

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

#endif

#ifdef _WIN32

    void ServerSocket::constructWifiSocket(const unsigned int& connectionBacklogSize)
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
        if (this->socketDescriptor == 0)
        {
             throw SocketException("Error establishing wifi server socket: " + std::string(std::strerror(errno)));
        }

        if (bind(this->socketDescriptor, this->serverAddress->ai_addr, (int)this->serverAddress->ai_addrlen) == -1) 
        {
            throw BindingException("Error binding connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)));
        }

        if(listen(this->socketDescriptor, connectionBacklogSize) == -1)
        {
            this->close();
            throw SocketException("Error Listening on port " + std::to_string(this->port) + ": " + std::string(std::strerror(errno)));
        }
    }

#elif __linux__

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

#endif

    /**
     * 
     * 
     * 
     */
    void ServerSocket::randomlyAllocatePort(const unsigned int &connectionBacklogSize)
    {
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
        
        std::uniform_int_distribution<> randDist(lowerBound, upperBound);

        // Only try to allocate the port 50 times, if it still fails then throw
        for (unsigned int i = 0; i < 50; i++)
        {
            try
            {
                this->port = randDist(rd);
                this->constructSocket(connectionBacklogSize);
                return;
            }
            catch (BindingException be)
            {
                // Nothing to do
            }
        }

        throw BindingException("Failed to randomly allocate port for ServerSocket after 50 attempts.");
    }

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

#endif

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
            timeOutVal.tv_usec = timeout;

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

#ifdef _WIN32

        int temp = ::accept(this->socketDescriptor, nullptr, nullptr);

#elif __linux__

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

    /**
     * Closes the existing connection. If no connection is open, then it will do nothing.
     * 
     * NOTE: This should be called before the server goes out of scope
     */
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
