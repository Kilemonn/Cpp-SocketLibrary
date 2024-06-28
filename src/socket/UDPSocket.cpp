#include "UDPSocket.h"

#include "../socketexceptions/SocketException.hpp"
#include "../socketexceptions/BindingException.hpp"

namespace kt
{
	UDPSocket::UDPSocket(const kt::UDPSocket& socket)
	{
		this->bound = socket.bound;
		this->receiveSocket = socket.receiveSocket;

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
	bool kt::UDPSocket::bind(const unsigned int& port, const kt::InternetProtocolVersion protocolVersion)
	{
		if (this->isUdpBound())
		{
			return true;
		}

#ifdef _WIN32
		WSADATA wsaData{};
		if (int res = WSAStartup(MAKEWORD(2, 2), &wsaData); res != 0)
		{
			throw kt::SocketException("WSAStartup Failed. " + std::to_string(res));
		}

#endif

		const int socketType = SOCK_DGRAM;
		const int socketProtocol = IPPROTO_UDP;

		addrinfo hints{};
		hints.ai_flags = AI_PASSIVE;
		hints.ai_family = static_cast<int>(protocolVersion);
		hints.ai_socktype = socketType;
		hints.ai_protocol = socketProtocol;

		std::pair<std::vector<kt::SocketAddress>, int> resolvedAddresses = kt::resolveToAddresses(std::nullopt, port, hints);
		if (resolvedAddresses.second != 0 || resolvedAddresses.first.empty())
		{
			throw kt::BindingException("Failed to resolve bind address with the provided port: " + std::to_string(port));
		}

		kt::SocketAddress firstAddress = resolvedAddresses.first.at(0);
		this->protocolVersion = static_cast<kt::InternetProtocolVersion>(firstAddress.address.sa_family);
		this->receiveSocket = socket(firstAddress.address.sa_family, socketType, socketProtocol);
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
		}
		else
		{
			this->listeningPort = std::make_optional(port);
		}

		return this->bound;
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
		int result = ::sendto(tempSocket, buffer, bufferLength, flags, &(address.address), sizeof(address));
		this->close(tempSocket);
		return std::make_pair(result != -1, result);
	}

	std::pair<bool, std::pair<int, kt::SocketAddress>> UDPSocket::sendTo(const std::string& hostname, const unsigned int& port, const std::string& message, const int& flags)
	{
		return this->sendTo(hostname, port, &message[0], message.size(), flags);
	}

	std::pair<bool, std::pair<int, kt::SocketAddress>> UDPSocket::sendTo(const std::string& hostname, const unsigned int& port, const char* buffer, const int& bufferLength, const int& flags)
	{
		addrinfo hints{};
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;

		std::pair<std::vector<kt::SocketAddress>, int> resolvedAddresses = kt::resolveToAddresses(std::make_optional(hostname), port, hints);
		if (resolvedAddresses.first.empty() || resolvedAddresses.second != 0)
		{
			return std::make_pair(false, std::make_pair(resolvedAddresses.second, kt::SocketAddress{}));
		}
		kt::SocketAddress firstAddress = resolvedAddresses.first.at(0);
		std::pair<bool, int> result = this->sendTo(buffer, bufferLength, firstAddress, flags);
		return std::make_pair(result.first, std::make_pair(result.second, firstAddress));
	}

	std::pair<std::optional<std::string>, std::pair<int, kt::SocketAddress>> UDPSocket::receiveFrom(const int& receiveLength, const int& flags)
	{
		std::string data;
		data.resize(receiveLength);

		std::pair<int, kt::SocketAddress> result = this->receiveFrom(&data[0], receiveLength, flags);
		
#ifdef _WIN32
		if (result.first < 1)
		{
			// This is for Windows, in some scenarios Windows will return a -1 flag but the buffer is populated properly
			// The code it is returning is 10040 this is indicating that the provided buffer is too small for the incoming
			// message, there is probably some settings we can tweak, however I think this is okay to return for now.
			return std::make_pair(data.empty() ? std::nullopt : std::make_optional(data), result);
		}
#endif

		// Need to substring to remove any null terminating bytes
		if (result.first < receiveLength)
		{
			data = data.substr(0, result.first);
		}
		
		return std::make_pair(data.empty() ? std::nullopt : std::make_optional(data), result);
	}

	std::pair<int, kt::SocketAddress> UDPSocket::receiveFrom(char* buffer, const int& receiveLength, const int& flags) const
	{
		kt::SocketAddress receiveAddress{};
		if (!this->bound || receiveLength == 0 || !this->ready())
		{
			return std::make_pair(0, receiveAddress);
		}

		auto addressLength = kt::getAddressLength(receiveAddress);
		int flag = recvfrom(this->receiveSocket, buffer, receiveLength, flags, &receiveAddress.address, &addressLength);
		return std::make_pair(flag, receiveAddress);
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

	std::optional<unsigned int> UDPSocket::getListeningPort() const
	{
		return this->listeningPort;
	}

	int UDPSocket::pollSocket(SOCKET socket, const long& timeout) const
	{
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

