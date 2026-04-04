
#include "TCPSocket.h"
#include "../socketexceptions/SocketException.hpp"

#include <cstring>

namespace kt
{
	TCPSocket::TCPSocket(const std::string& hostname, const unsigned short& port, const kt::InternetProtocolVersion protocolVersion)
	{
		this->hostname = hostname;
		this->port = port;
		this->protocolVersion = protocolVersion;

		constructSocket();
	}

	TCPSocket::TCPSocket(const SOCKET& socket, const std::string& hostname, const unsigned short& port, const kt::InternetProtocolVersion protocolVersion, const kt::SocketAddress& acceptedAddress)
	{
		this->socketDescriptor = socket;
		this->hostname = hostname;
		this->port = port;
		this->protocolVersion = protocolVersion;
		this->serverAddress = acceptedAddress;
	}

    TCPSocket::TCPSocket(const kt::SocketAddress address)
    {
		std::optional<std::string> resolvedHostname = kt::getAddress(address);
		this->hostname = resolvedHostname.value_or("");
		this->port = kt::getPortNumber(address);
		this->protocolVersion = kt::getInternetProtocolVersion(address);

		addrinfo hints = kt::createTcpHints();
		this->socketDescriptor = socket(address.address.sa_family, hints.ai_socktype, hints.ai_protocol);
		if (isInvalidSocket(this->socketDescriptor))
		{
			throw kt::SocketException("Unable to construct socket to provided addresses with hostname [" + this->hostname + ":" + std::to_string(this->port) + "] " + getErrorCode());
		}

		int connectionResult = connect(this->socketDescriptor, &address.address, sizeof(address));
		if (connectionResult == 0)
		{
			this->serverAddress = address;
			this->protocolVersion = static_cast<kt::InternetProtocolVersion>(address.address.sa_family);
		}
		else
		{
			throw kt::SocketException("Unable to connect to provided address with hostname [" + this->hostname + ":" + std::to_string(this->port) + "] " + getErrorCode());
		}
    }

    TCPSocket::TCPSocket(const kt::TCPSocket& socket)
	{
		this->socketDescriptor = socket.socketDescriptor;
		this->hostname = socket.hostname;
		this->port = socket.port;
		this->protocolVersion = socket.protocolVersion;
		this->serverAddress = socket.serverAddress;
	}

	TCPSocket& TCPSocket::operator=(const kt::TCPSocket& socket)
	{
		this->socketDescriptor = socket.socketDescriptor;
		this->hostname = socket.hostname;
		this->port = socket.port;
		this->protocolVersion = socket.protocolVersion;
		this->serverAddress = socket.serverAddress;

		return *this;
	}

	void TCPSocket::constructSocket()
	{
#ifdef _WIN32
		WSADATA wsaData{};
		if (int res = WSAStartup(MAKEWORD(2, 2), &wsaData); res != 0)
		{
			throw kt::SocketException("WSAStartup Failed. " + std::to_string(res));
		}

#endif

		addrinfo hints = kt::createTcpHints(this->protocolVersion);
		std::pair<std::vector<kt::SocketAddress>, int> addresses = kt::resolveToAddresses(this->hostname, this->port, hints);
		if (addresses.second != 0)
		{
			// std::cout << "Look up response code: [" <<gai_strerror(addresses.second) << "]" << std::endl;
			throw kt::SocketException("Unable to resolve IP of destination address with hostname: [" + this->hostname + ":" + std::to_string(this->port) + "]. Look up response code: [" + std::to_string(addresses.second) + "]. " + getErrorCode());
		}

		// We need to iterate over the resolved address and attempt to connect to each of them, if a connection attempt is succesful 
		// we will return, otherwise we will throw is we are unable to connect to any.
		for (kt::SocketAddress address : addresses.first)
		{
			this->socketDescriptor = socket(address.address.sa_family, hints.ai_socktype, hints.ai_protocol);
			if (!isInvalidSocket(this->socketDescriptor))
			{
				int connectionResult = connect(this->socketDescriptor, &address.address, sizeof(address));
				if (connectionResult == 0)
				{
					this->serverAddress = address;
					this->protocolVersion = static_cast<kt::InternetProtocolVersion>(address.address.sa_family);
					// Return once we successfully connect to one
					return;
				}
			}

			this->close();
		}
		
		throw kt::SocketException("Unable to connect to resolved addresses for provided hostname [" + this->hostname + ":" + std::to_string(this->port) + "] " + getErrorCode());
	}

	void TCPSocket::close()
	{
		Socket::close(this->socketDescriptor);
		this->socketDescriptor = getInvalidSocketValue();
	}

    SOCKET TCPSocket::getSocket() const
    {
        return this->socketDescriptor;
    }

    std::string TCPSocket::getHostname() const
    {
		return this->hostname;
	}

	unsigned short TCPSocket::getPort() const
	{
		return this->port;
	}

	kt::InternetProtocolVersion TCPSocket::getInternetProtocolVersion() const
	{
		return this->protocolVersion;
	}

	kt::SocketAddress TCPSocket::getSocketAddress() const
	{
		return this->serverAddress;
	}
}
