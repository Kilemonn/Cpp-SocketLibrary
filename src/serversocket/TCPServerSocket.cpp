
#include "TCPServerSocket.h"
#include "../socket/Socket.h"
#include "../socketexceptions/SocketException.hpp"
#include "../socketexceptions/BindingException.hpp"
#include "../socketexceptions/TimeoutException.hpp"
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

#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>

#endif

namespace kt
{
    /**
     * TCPServerSocket constructor. Creates a wifi TCPServerSocket and begins listening for connections.
     *
     * @param port - The port number for this server to communicate through. If value is not passed in a random, available port number will be assigned.
     * @param connectionBacklogSize - You can enter a value here to specify the length of the server connection pool. The default value is 20.
     * 
     * @throw SocketException - If the TCPServerSocket is unable to be instanciated or begin listening.
     * @throw BindingException - If the TCPServerSocket is unable to bind to the specific port specified.
     */
    kt::TCPServerSocket::TCPServerSocket(const std::optional<std::string>& localHostname, const unsigned short& port, const unsigned int& connectionBacklogSize, const kt::InternetProtocolVersion protocolVersion, const std::optional<std::function<void(SOCKET&)>>& preBindSocketOperation)
    {
        this->socketDescriptor = getInvalidSocketValue();
        this->port = port;
        this->protocolVersion = protocolVersion;

        this->constructSocket(localHostname, connectionBacklogSize, preBindSocketOperation);
    }

    /**
     * ServerSocket copy constructor.
     * 
     * @param socket - The TCPServerSocket object to be copied.
     */
    kt::TCPServerSocket::TCPServerSocket(const kt::TCPServerSocket& socket)
    {
        this->port = socket.port;
        this->protocolVersion = socket.protocolVersion;
        this->socketDescriptor = socket.socketDescriptor;
        this->serverAddress = socket.serverAddress;
    }

    /**
     * Overloaded assignment operator for the TCPServerSocket class.
     * 
     * @param socket - The TCPServerSocket object to be copied.
     * 
     * @return the copied socket
     */
    kt::TCPServerSocket& kt::TCPServerSocket::operator=(const kt::TCPServerSocket& socket)
    {
        this->port = socket.port;
        this->protocolVersion = socket.protocolVersion;
        this->socketDescriptor = socket.socketDescriptor;
        this->serverAddress = socket.serverAddress;

        return *this;
    }

    void kt::TCPServerSocket::constructSocket(const std::optional<std::string>& localHostname, const unsigned int& connectionBacklogSize, const std::optional<std::function<void(SOCKET&)>>& preBindSocketOperation)
    {

#ifdef _WIN32
        WSADATA wsaData{};
        if (int res = WSAStartup(MAKEWORD(2, 2), &wsaData); res != 0)
        {
            throw kt::SocketException("WSAStartup Failed: " + std::to_string(res));
        }
#endif

        addrinfo hints = kt::createTcpHints(this->protocolVersion, AI_PASSIVE);
        std::pair<std::vector<kt::SocketAddress>, int> resolveAddresses = kt::resolveToAddresses(localHostname.has_value() ? localHostname.value().c_str() : kt::getLocalAddress(protocolVersion), this->port, hints);

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
            throw kt::SocketException("Error establishing TCP server socket: " + getErrorCode());
        }

#if defined(__linux__) || defined(__APPLE__)
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
        if (preBindSocketOperation.has_value())
        {
            preBindSocketOperation.value()(this->socketDescriptor);
        }

        socklen_t socketSize = kt::getAddressLength(serverAddress);
        if (bind(this->socketDescriptor, &this->serverAddress.address, socketSize) == -1)
        {
            this->close();
            throw kt::BindingException("Error binding connection, the port " + std::to_string(this->port) + " is already being used: " + getErrorCode());
        }

        if (this->port == 0)
        {
            this->initialisePortNumber();
            this->serverAddress.ipv4.sin_port = htons(this->port);
        }

        if (listen(this->socketDescriptor, connectionBacklogSize) == -1)
        {
            this->close();
            throw kt::SocketException("Error Listening on port " + std::to_string(this->port) + ": " + getErrorCode());
        }
    }

    void kt::TCPServerSocket::initialisePortNumber()
    {
        std::pair<std::optional<kt::SocketAddress>, int> address = kt::socketToAddress(this->socketDescriptor);
		if (address.second != 0 && !address.first.has_value())
		{
			this->close();
			throw kt::BindingException("Unable to retrieve randomly bound port number during socket creation. " + getErrorCode());
		}

		this->port = kt::getPortNumber(address.first.value());
    }

    /**
     * Used to get the port number that the TCPServerSocket is listening on.
     * @return An unsigned int of the port number that the TCPServerSocket is listening on.
     */
    unsigned short kt::TCPServerSocket::getPort() const
    {
        return this->port;
    }

    SOCKET TCPServerSocket::getSocket() const
    {
        return this->socketDescriptor;
    }

    kt::SocketAddress TCPServerSocket::getSocketAddress() const
    {
        return this->serverAddress;
    }

    /**
     * @return the *kt::InternetProtocolVersion* for this *kt::TCPServerSocket*.
     */
    kt::InternetProtocolVersion kt::TCPServerSocket::getInternetProtocolVersion() const
    {
        return this->protocolVersion;
    }

    kt::TCPSocket kt::TCPServerSocket::accept(const long& timeout) const
    {
        if (timeout > 0)
        {
            int res = Socket::pollSocket(this->socketDescriptor, timeout);
            if (res == -1)
            {
                throw kt::SocketException("Failed to poll as socket is no longer valid: " + kt::getErrorCode() + "(" + std::to_string(errno) + ")");
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

    /**
     * Closes the existing connection. If no connection is open, then it will do nothing.
     * 
     * NOTE: This should be called before the server goes out of scope
     */
    void TCPServerSocket::close()
    {
        Socket::close(this->socketDescriptor);
        this->socketDescriptor = getInvalidSocketValue();
    }

} // End namespace kt
