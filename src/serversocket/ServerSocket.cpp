
#include "ServerSocket.h"
#include "../socket/Socket.h"
#include "../socketexceptions/SocketException.hpp"
#include "../socketexceptions/BindingException.hpp"
#include "../socketexceptions/TimeoutException.hpp"
#include "../enums/SocketType.h"
#include "../socketexceptions/SocketError.h"
#include "../address/SocketAddress.h"

#include <iostream>
#include <cstdlib>
#include <random>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <string>

#ifdef _WIN32

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
    kt::ServerSocket::ServerSocket(const kt::SocketType type, const unsigned short& port, const unsigned int& connectionBacklogSize, const kt::InternetProtocolVersion protocolVersion)
    {
        this->socketDescriptor = getInvalidSocketValue();
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
    kt::ServerSocket::ServerSocket(const kt::ServerSocket& socket)
    {
        this->port = socket.port;
        this->type = socket.type;
        this->protocolVersion = socket.protocolVersion;
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
    kt::ServerSocket& kt::ServerSocket::operator=(const kt::ServerSocket& socket)
    {
        this->port = socket.port;
        this->type = socket.type;
        this->protocolVersion = socket.protocolVersion;
        this->socketDescriptor = socket.socketDescriptor;
        this->serverAddress = socket.serverAddress;

        return *this;
    }

    void kt::ServerSocket::constructSocket(const unsigned int& connectionBacklogSize)
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

    void kt::ServerSocket::constructBluetoothSocket(const unsigned int& connectionBacklogSize)
    {
#ifdef _WIN32
        throw kt::SocketException("ServerSocket::constructBluetoothSocket(unsigned int) is not supported on Windows.");

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

    void kt::ServerSocket::constructWifiSocket(const unsigned int& connectionBacklogSize)
    {

#ifdef _WIN32
        WSADATA wsaData{};
        if (int res = WSAStartup(MAKEWORD(2, 2), &wsaData); res != 0)
        {
            throw kt::SocketException("WSAStartup Failed: " + std::to_string(res));
        }
#endif

        addrinfo hints = kt::createTcpHints(this->protocolVersion, AI_PASSIVE);
        std::pair<std::vector<kt::SocketAddress>, int> resolveAddresses = kt::resolveToAddresses(kt::getLocalAddress(protocolVersion), this->port, hints);

        if (resolveAddresses.second != 0 || resolveAddresses.first.empty())
        {
            throw kt::SocketException("Failed to retrieve address info of local hostname. " + getErrorCode());
        }
        kt::SocketAddress address = resolveAddresses.first.at(0);
        this->protocolVersion = static_cast<kt::InternetProtocolVersion>(address.address.sa_family);
        this->serverAddress = address;

        this->socketDescriptor = socket(static_cast<int>(this->protocolVersion), hints.ai_socktype, hints.ai_protocol);
        if (isInvalidSocket(this->socketDescriptor))
        {
            throw kt::SocketException("Error establishing wifi server socket: " + getErrorCode());
        }

#ifdef __linux__
        const int enableOption = 1;
        if (setsockopt(this->socketDescriptor, SOL_SOCKET, SO_REUSEADDR, (const char*)&enableOption, sizeof(enableOption)) != 0)
        {
            throw SocketException("Failed to set SO_REUSEADDR socket option: " + getErrorCode());
        }
#endif

#ifdef _WIN32
        if (this->protocolVersion == kt::InternetProtocolVersion::IPV6)
        {
            const int disableOption = 0;
            if (setsockopt(this->socketDescriptor, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&disableOption, sizeof(disableOption)) != 0)
            {
                throw kt::SocketException("Failed to set IPV6_V6ONLY socket option: " + getErrorCode());
            }
        }
#endif
        socklen_t socketSize = sizeof(this->serverAddress);
        if (bind(this->socketDescriptor, &this->serverAddress.address, socketSize) == -1)
        {
            this->close();
            throw kt::BindingException("Error binding connection, the port " + std::to_string(this->port) + " is already being used: " + getErrorCode());
        }

        if (this->port == 0)
        {
            this->initialisePortNumber();
        }

        if (listen(this->socketDescriptor, connectionBacklogSize) == -1)
        {
            this->close();
            throw kt::SocketException("Error Listening on port " + std::to_string(this->port) + ": " + getErrorCode());
        }
    }

    void kt::ServerSocket::initialisePortNumber()
    {
        std::pair<std::optional<kt::SocketAddress>, int> address = kt::socketToAddress(this->socketDescriptor);
		if (address.second != 0 && !address.first.has_value())
		{
			this->close();
			throw kt::BindingException("Unable to retrieve randomly bound port number during socket creation. " + getErrorCode());
		}

		this->port = kt::getPortNumber(address.first.value());
    }


    void kt::ServerSocket::setDiscoverable()
    {
        throw kt::SocketException("ServerSocket::setDiscoverable() not implemented.");

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
     * @return the *kt::SocketType* for this *kt::ServerSocket*.
     */
    kt::SocketType kt::ServerSocket::getType() const
    {
        return this->type;
    }

    /**
     * Used to get the port number that the ServerSocket is listening on.
     * @return An unsigned int of the port number that the ServerSocket is listening on.
     */
    unsigned short kt::ServerSocket::getPort() const
    {
        return this->port;
    }

    SOCKET ServerSocket::getSocket() const
    {
        return this->socketDescriptor;
    }

    kt::SocketAddress ServerSocket::getSocketAddress() const
    {
        return this->serverAddress;
    }

    /**
     * @return the *kt::InternetProtocolVersion* for this *kt::ServerSocket*.
     */
    kt::InternetProtocolVersion kt::ServerSocket::getInternetProtocolVersion() const
    {
        return this->protocolVersion;
    }

    kt::TCPSocket kt::ServerSocket::acceptTCPConnection(const long& timeout) const
    {
        if (timeout > 0)
        {
            int res = kt::pollSocket(this->socketDescriptor, timeout);
            if (res == -1)
            {
                throw kt::SocketException("Failed to poll as socket is no longer valid.");
            }
            else if (res == 0)
            {
                throw kt::TimeoutException("No applicable connections could be accepted during the time period specified " + std::to_string(timeout) + " microseconds.");
            }
        }

        kt::SocketAddress acceptedAddress{};
        socklen_t sockLen = sizeof(acceptedAddress);
        SOCKET temp = ::accept(this->socketDescriptor, &acceptedAddress.address, &sockLen);
        if (isInvalidSocket(temp))
        {
            throw kt::SocketException("Failed to accept connection. Socket is in an invalid state.");
        }

        unsigned int portNum = this->getInternetProtocolVersion() == kt::InternetProtocolVersion::IPV6 ? htons(acceptedAddress.ipv6.sin6_port) : htons(acceptedAddress.ipv4.sin_port);
        std::optional<std::string> hostname = kt::getAddress(acceptedAddress);
		if (!hostname.has_value())
		{
            throw kt::SocketException("Unable to resolve accepted hostname from accepted socket.");
		}

        return kt::TCPSocket(temp, hostname.value(), portNum, this->getInternetProtocolVersion(), acceptedAddress);
    }

    kt::BluetoothSocket kt::ServerSocket::acceptBluetoothConnection(const long& timeout)
    {
        if (timeout > 0)
        {
            int res = kt::pollSocket(this->socketDescriptor, timeout);
            if (res == -1)
            {
                throw kt::SocketException("Failed to poll as socket is no longer valid.");
            }
            else if (res == 0)
            {
                throw kt::TimeoutException("No applicable connections could be accepted during the time period specified " + std::to_string(timeout) + " microseconds.");
            }
        }

        throw kt::SocketException("acceptBluetoothConnection() - Not yet implemented.");
#ifdef __linux__
        // Remove bluetooth related code

        // sockaddr_rc remoteDevice = { 0 };
        // socklen_t socketSize = sizeof(remoteDevice);
        // SOCKET temp = ::accept(this->socketDescriptor, (sockaddr *) &remoteDevice, &socketSize);
        // if (temp == -1)
        // {
        //     throw SocketException("Failed to accept connection. Socket is in an invalid state.");
        // }
        
        // if (this->type == kt::SocketType::Bluetooth)
        // {
        // 	char remoteAddress[1024] = {0};
	    //     ba2str(&remoteDevice.rc_bdaddr, remoteAddress);
        // }
#endif
    }

    /**
     * Closes the existing connection. If no connection is open, then it will do nothing.
     * 
     * NOTE: This should be called before the server goes out of scope
     */
    void ServerSocket::close()
    {
        kt::close(this->socketDescriptor);
    }

} // End namespace kt
