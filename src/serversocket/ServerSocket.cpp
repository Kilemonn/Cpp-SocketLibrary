
#include "ServerSocket.h"
#include "../socket/Socket.h"
#include "../socketexceptions/SocketException.hpp"
#include "../socketexceptions/BindingException.hpp"
#include "../socketexceptions/TimeoutException.hpp"
#include "../enums/SocketProtocol.h"
#include "../enums/SocketType.h"
#include "../socketexceptions/SocketError.h"

#include <iostream>
#include <cstdlib>
#include <random>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <string>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define _WIN32_WINNT 0x501

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#include <guiddef.h>
#include <ws2bth.h>
#include <winerror.h>

#pragma comment(lib, "ws2_32.lib")

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
     * ServerSocket constructor. Creates a wifi/bluetooth ServerSocket and begins listening for connections.
     *
     * @param type - Determines whether this ServerSocket is a wifi or bluetooth ServerSocket.
     * @param port - The port number for this server to communicate through. If value is not passed in a random, available port number will be assigned.
     * @param connectionBacklogSize - You can enter a value here to specify the length of the server connection pool. The default value is 20.
     * 
     * @throw SocketException - If the ServerSocket is unable to be instanciated or begin listening.
     * @throw BindingException - If the ServerSocket is unable to bind to the specific port specified.
     */
    ServerSocket::ServerSocket(const kt::SocketType type, const unsigned int& port, const unsigned int& connectionBacklogSize, const InternetProtocolVersion protocolVersion)
    {
        this->port = port;
        this->type = type;
        this->protocolVersion = protocolVersion;

        if (this->type == kt::SocketType::None)
        {
            throw SocketException("Failed to create ServerSocket with 'None' SocketType.");
        }

        this->constructSocket(connectionBacklogSize);
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
#ifdef _WIN32
        throw SocketException("ServerSocket::constructBluetoothSocket(unsigned int) is not supported on Windows.");

        /*SOCKADDR_BTH bluetoothAddress;

        this->socketDescriptor = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

        if (this->socketDescriptor == 0)
        {
            throw SocketException("Error establishing BT server socket: " + std::string(std::strerror(errno)));
        }

        this->bluetoothAddress.addressFamily = AF_BTH;
        this->bluetoothAddress.btAddr = 0;
        this->bluetoothAddress.port = this->port;

        if (bind(this->socketDescriptor, (sockaddr*)&this->bluetoothAddress, sizeof(SOCKADDR_BTH)) == -1)
        {
            throw BindingException("Error binding BT connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)) + ". WSA Error: " + std::to_string(WSAGetLastError()));
        }

        if (listen(this->socketDescriptor, static_cast<int>(connectionBacklogSize)) == -1)
        {
            this->close();
            throw SocketException("Error Listening on port: " + std::to_string(this->port) + ": " + std::string(std::strerror(errno)));
        }*/

#elif __linux__
        sockaddr_rc localAddress = {0};
        this->socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

        if (isInvalidSocket(this->socketDescriptor))
        {
            throw SocketException("Error establishing BT server socket: " + std::string(std::strerror(errno)));
        }

        localAddress.rc_family = AF_BLUETOOTH;
        localAddress.rc_bdaddr = ((bdaddr_t) {{0, 0, 0, 0, 0, 0}});
        localAddress.rc_channel = static_cast<uint8_t>(this->port);
        
        if (bind(this->socketDescriptor, (sockaddr *)&localAddress, sizeof(localAddress)) == -1)
        {
            throw BindingException("Error binding BT connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)));
        }

        if (listen(this->socketDescriptor, connectionBacklogSize) == -1)
        {
            this->close();
            throw SocketException("Error Listening on port " + std::to_string(this->port) + ": " + std::string(std::strerror(errno)));
        }
#endif
        
        // Make discoverable

    }

    void ServerSocket::constructWifiSocket(const unsigned int& connectionBacklogSize)
    {
        const int socketFamily = this->protocolVersion == InternetProtocolVersion::IPV6 ? AF_INET6 : AF_INET;
        const int socketType = SOCK_STREAM;
        const int socketProtocol = IPPROTO_TCP;

#ifdef _WIN32
        WSADATA wsaData;
        if (int res = WSAStartup(MAKEWORD(2, 2), &wsaData); res != 0)
        {
            throw SocketException("WSAStartup Failed: " + std::to_string(res));
        }
#endif
        /*addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = socketFamily;
        hints.ai_socktype = socketType;
        hints.ai_protocol = socketProtocol;
        hints.ai_flags = AI_PASSIVE;*/

        memset(&this->serverAddress, 0, sizeof(this->serverAddress));
        this->serverAddress.sin6_family = socketFamily;
        this->serverAddress.sin6_addr = in6addr_loopback;
        this->serverAddress.sin6_port = htons(this->port);

        this->socketDescriptor = socket(socketFamily, socketType, socketProtocol);
        if (isInvalidSocket(this->socketDescriptor))
        {
            //freeaddrinfo(resolvedAddresses);
            throw SocketException("Error establishing wifi server socket: " + getErrorCode());
        }
        else
        {
            std::cout << "Created socket is good..." << std::endl;
        }

        const int enableOption = 1;
        if (setsockopt(this->socketDescriptor, SOL_SOCKET, SO_REUSEADDR, (const char*)&enableOption, sizeof(enableOption)) != 0)
        {
            //freeaddrinfo(resolvedAddresses);
            throw SocketException("Failed to set SO_REUSEADDR socket option: " + getErrorCode());
        }

        const int disableOption = 0;
        if (setsockopt(this->socketDescriptor, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&disableOption, sizeof(disableOption)) != 0)
        {
            //freeaddrinfo(resolvedAddresses);
            throw SocketException("Failed to set IPV6_V6ONLY socket option: " + getErrorCode());
        }

        // std::memcpy(&this->serverAddress, addr->ai_addr, sizeof(addr->ai_addrlen));
        if (bind(this->socketDescriptor, (sockaddr*)&this->serverAddress, sizeof(this->serverAddress)) == -1)
        {
            //freeaddrinfo(resolvedAddresses);
            this->close();
            throw BindingException("Error binding connection, the port " + std::to_string(this->port) + " is already being used: " + getErrorCode());
        }
        else
        {
            std::cout << "Bind is good..." << std::endl;
        }

        socklen_t socketSize = sizeof(this->serverAddress);
        if (this->port == 0)
        {
            if (getsockname(this->socketDescriptor, (sockaddr*)&this->serverAddress, &socketSize) != 0)
            {
                //freeaddrinfo(resolvedAddresses);
                this->close();
                throw BindingException("Unable to retrieve randomly bound port number during socket creation. " + getErrorCode());
            }

            this->port = ntohs(this->serverAddress.sin6_port);
        }

        if (listen(this->socketDescriptor, connectionBacklogSize) == -1)
        {
            //freeaddrinfo(resolvedAddresses);
            this->close();
            throw SocketException("Error Listening on port " + std::to_string(this->port) + ": " + getErrorCode());
        }
        else
        {
            std::cout << "Listen successful" << std::endl;
            // freeaddrinfo(resolvedAddresses);
            return;
        }

        /*addrinfo* resolvedAddresses = nullptr;
        int res = getaddrinfo(nullptr, std::to_string(this->port).c_str(), &hints, &resolvedAddresses);
        if (res != 0)
        {
            throw SocketException("Failed to setup socket server: " + getErrorCode());
        }

        addrinfo* addr = resolvedAddresses;

        while (addr != nullptr)
        {
            
            
            addr = addr->ai_next;
            std::cout << "Moving to next address" << std::endl;
        }
        std::cout << "Iterated through all addresses" << std::endl;
        freeaddrinfo(resolvedAddresses);*/
    }


    void ServerSocket::setDiscoverable()
    {
        throw SocketException("ServerSocket::setDiscoverable() not implemented.");

#if __linux__
        hci_dev_req req;
        req.dev_id = 0;
        // req.dev_id = hci_get_route(nullptr);
        req.dev_opt = SCAN_PAGE | SCAN_INQUIRY;

        if (ioctl(this->socketDescriptor, HCISETSCAN, (unsigned long)&req) < 0)
        {
            throw SocketException("Failed to make device discoverable.");            
        }
#endif
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
            timeval timeOutVal;
            memset((char *)&timeOutVal, '\0', sizeof(timeOutVal));
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

#ifdef _WIN32
        SOCKET temp = ::accept(this->socketDescriptor, nullptr, nullptr);
        if (temp == -1)
        {
            throw SocketException("Failed to accept connection. Socket is in an invalid state.");
        }

#elif __linux__
        sockaddr_rc remoteDevice = { 0 };
        socklen_t socketSize = sizeof(remoteDevice);
        int temp = ::accept(this->socketDescriptor, (sockaddr *) &remoteDevice, &socketSize);
        if (temp == -1)
        {
            throw SocketException("Failed to accept connection. Socket is in an invalid state.");
        }
        
        if (this->type == kt::SocketType::Bluetooth)
        {
        	char remoteAddress[1024] = {0};
	        ba2str(&remoteDevice.rc_bdaddr, remoteAddress);
        }
#endif

        sockaddr_in address{};
		socklen_t addr_size = sizeof(address);
		int res = getpeername(temp, (sockaddr*)&address, &addr_size);

        std::string hostname;
        unsigned int portNum = 0;
		if (res == 0)
		{ 
			hostname = std::string(inet_ntoa(address.sin_addr));
            portNum = htons(address.sin_port);
		}

        return Socket(temp, this->type, kt::SocketProtocol::TCP, hostname, portNum);
    }

    /**
     * Closes the existing connection. If no connection is open, then it will do nothing.
     * 
     * NOTE: This should be called before the server goes out of scope
     */
    void ServerSocket::close()
    {
#ifdef _WIN32
        closesocket(this->socketDescriptor);

#elif __linux__
        ::close(this->socketDescriptor);

#endif
    }

} // End namespace kt
