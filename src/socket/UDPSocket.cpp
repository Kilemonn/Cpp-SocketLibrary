#include "UDPSocket.h"

#include "../socketexceptions/SocketException.hpp"
#include "../socketexceptions/BindingException.hpp"

namespace kt
{
	UDPSocket::UDPSocket(const kt::UDPSocket& socket)
	{
		this->bound = socket.bound;
		this->receiveSocket = socket.receiveSocket;
		this->listeningPort = socket.listeningPort;
		this->protocolVersion = socket.protocolVersion;

#ifdef _WIN32
		WSADATA wsaData{};
		if (int res = WSAStartup(MAKEWORD(2, 2), &wsaData); res != 0)
		{
			throw kt::SocketException("WSAStartup Failed. " + std::to_string(res));
		}

#endif
	}

	kt::UDPSocket& UDPSocket::operator=(const kt::UDPSocket& socket)
	{
		this->bound = socket.bound;
		this->receiveSocket = socket.receiveSocket;
		this->listeningPort = socket.listeningPort;
		this->protocolVersion = socket.protocolVersion;

		return *this;
	}

	/**
	 * This method is required for kt::SocketProtocol::UDP sockets.
	 * The socket that is listening for new connections will need to call this before they begin listening (accepting connections).
	 * This ensures the socket is bound to the port and can receive new connections. *Only a single process can be bound to a single port at one time*.
	 *
	 * @return bool - true if the socket was bound successfully, otherwise false
	 *
	 * @throw BindingException - if the socket fails to bind
	 */
	std::pair<bool, kt::SocketAddress> kt::UDPSocket::bind(const unsigned short& port, const kt::InternetProtocolVersion protocolVersion)
	{
		if (this->isUdpBound())
		{
			this->close();
		}

#ifdef _WIN32
		WSADATA wsaData{};
		if (int res = WSAStartup(MAKEWORD(2, 2), &wsaData); res != 0)
		{
			throw kt::SocketException("WSAStartup Failed. " + std::to_string(res));
		}

#endif

		addrinfo hints = kt::createUdpHints(protocolVersion, AI_PASSIVE);
		std::pair<std::vector<kt::SocketAddress>, int> resolvedAddresses = kt::resolveToAddresses(kt::getLocalAddress(protocolVersion), port, hints);
		if (resolvedAddresses.second != 0 || resolvedAddresses.first.empty())
		{
			throw kt::BindingException("Failed to resolve bind address with the provided port: " + std::to_string(port));
		}

		kt::SocketAddress firstAddress = resolvedAddresses.first.at(0);
		this->protocolVersion = static_cast<kt::InternetProtocolVersion>(firstAddress.address.sa_family);
		this->receiveSocket = socket(firstAddress.address.sa_family, hints.ai_socktype, hints.ai_protocol);
		if (kt::isInvalidSocket(this->receiveSocket))
		{
			throw kt::SocketException("Unable to construct socket from local host details. " + getErrorCode());
		}

#ifdef _WIN32
		if (this->protocolVersion == kt::InternetProtocolVersion::IPV6)
		{
			const int disableOption = 0;
			if (setsockopt(this->receiveSocket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&disableOption, sizeof(disableOption)) != 0)
			{
				throw kt::SocketException("Failed to set IPV6_V6ONLY socket option: " + getErrorCode());
			}
		}

#endif

		int bindResult = ::bind(this->receiveSocket, &firstAddress.address, kt::getAddressLength(firstAddress));
		this->bound = bindResult != -1;
		if (!this->bound)
		{
			throw kt::BindingException("Error binding connection, the port " + std::to_string(port) + " is already being used. Response code from ::bind()" + std::to_string(bindResult) + ". Latest Error code: " + getErrorCode());
		}

		if (port == 0)
		{
			this->initialiseListeningPortNumber();
			firstAddress.ipv4.sin_port = htons(this->listeningPort.value());
		}
		else
		{
			this->listeningPort = std::make_optional(port);
		}

		return std::make_pair(this->bound, firstAddress);
	}

	void UDPSocket::close()
	{
		this->close(this->receiveSocket);
		
		this->bound = false;
		this->listeningPort = std::nullopt;
	}

	bool UDPSocket::ready(const unsigned long timeout) const
	{
		int result = this->pollSocket(this->receiveSocket, timeout);
		// 0 indicates that there is no data
		return result > 0;
	}

	std::pair<bool, int> UDPSocket::sendTo(const std::string& message, const kt::SocketAddress& address, const int& flags)
	{
		return this->sendTo(&message[0], message.size(), address, flags);
	}

	std::pair<bool, int> UDPSocket::sendTo(const char* buffer, const int& bufferLength, const kt::SocketAddress& address, const int& flags)
	{
		SOCKET tempSocket = socket(address.address.sa_family, SOCK_DGRAM, IPPROTO_UDP);
		if (kt::isInvalidSocket(tempSocket))
		{
			return std::make_pair(false, -2);
		}

		if (preSendSocketOperation.has_value())
		{
			preSendSocketOperation.value()(tempSocket);
		}

		int result = ::sendto(tempSocket, buffer, bufferLength, flags, &(address.address), sizeof(address));
		this->close(tempSocket);
		return std::make_pair(result != -1, result);
	}

	std::pair<std::pair<bool, int>, kt::SocketAddress> UDPSocket::sendTo(const std::string& hostname, const unsigned short& port, const std::string& message, const int& flags, const kt::InternetProtocolVersion protocolVersion)
	{
		return this->sendTo(hostname, port, &message[0], message.size(), flags, protocolVersion);
	}

	std::pair<std::pair<bool, int>, kt::SocketAddress> UDPSocket::sendTo(const std::string& hostname, const unsigned short& port, const char* buffer, const int& bufferLength, const int& flags, const kt::InternetProtocolVersion protocolVersion)
	{
		addrinfo hints = kt::createUdpHints(protocolVersion);
		std::pair<std::vector<kt::SocketAddress>, int> resolvedAddresses = kt::resolveToAddresses(hostname, port, hints);
		if (resolvedAddresses.first.empty() || resolvedAddresses.second != 0)
		{
			return std::make_pair(std::make_pair(false, resolvedAddresses.second), kt::SocketAddress{});
		}
		kt::SocketAddress firstAddress = resolvedAddresses.first.at(0);
		std::pair<bool, int> result = this->sendTo(buffer, bufferLength, firstAddress, flags);
		return std::make_pair(result, firstAddress);
	}

	std::pair<std::optional<std::string>, std::pair<int, kt::SocketAddress>> UDPSocket::receiveFrom(const int& receiveLength, const int& flags)
	{
		std::string data;
		data.resize(receiveLength);

		std::pair<int, kt::SocketAddress> result = this->receiveFrom(&data[0], receiveLength, flags);

		// Need to substring to remove any null terminating bytes
		if (result.first >= 0 && result.first < receiveLength)
		{
			data = data.substr(0, result.first);
		}

		return std::make_pair(data.size() == 0 ? std::nullopt : std::make_optional(data), result);
	}

	std::pair<int, kt::SocketAddress> UDPSocket::receiveFrom(char* buffer, const int& receiveLength, const int& flags) const
	{
		kt::SocketAddress receiveAddress{};
		if (!this->bound || receiveLength == 0 || !this->ready())
		{
			return std::make_pair(-1, receiveAddress);
		}

		// Using auto here since the "addressLength" argument for "::recvfrom()" has differing types depending what platform
		// we are on, so I am letting the definition of kt::getAddressLength() drive this type via auto
		auto addressLength = kt::getAddressLength(receiveAddress);

		// In some scenarios Windows will return a -1 flag value but the buffer is populated properly with the correct length
		// The code it is returning is 10040 this is indicating that the provided buffer is too small for the incoming
		// message, there is probably some settings we can tweak, however I think this is okay to return for now.
		int flag = ::recvfrom(this->receiveSocket, buffer, receiveLength, flags, &receiveAddress.address, &addressLength);
		return std::make_pair(flag, receiveAddress);
	}

    SOCKET UDPSocket::getListeningSocket() const
    {
        return this->receiveSocket;
    }

    bool UDPSocket::isUdpBound() const
    {
		return this->bound;
	}

	/**
	* This is only applicable and useful if the UDPSocket is isUdpBound() returns *true*.
	*/
	kt::InternetProtocolVersion UDPSocket::getInternetProtocolVersion() const
	{
		return this->protocolVersion;
	}

	std::optional<unsigned short> UDPSocket::getListeningPort() const
	{
		return this->listeningPort;
	}

    void UDPSocket::setPreSendSocketOperation(std::function<void(SOCKET&)> newOperation)
    {
		this->preSendSocketOperation = std::make_optional(newOperation);
    }

    int UDPSocket::pollSocket(SOCKET socket, const long& timeout) const
	{
		if (kt::isInvalidSocket(socket))
		{
			return -1;
		}

		timeval timeOutVal{};
		int res = kt::pollSocket(socket, timeout, &timeOutVal);
		return res;
	}

	void UDPSocket::initialiseListeningPortNumber()
	{
		std::pair<std::optional<kt::SocketAddress>, int> address = kt::socketToAddress(this->receiveSocket);
		if (address.second != 0 && !address.first.has_value())
		{
			this->close();
			throw kt::BindingException("Unable to retrieve randomly bound port number during socket creation. " + getErrorCode());
		}

		this->listeningPort = std::make_optional(kt::getPortNumber(address.first.value()));
	}

	void UDPSocket::close(SOCKET socket)
	{
		kt::close(socket);
	}
	
}

