
#include "TCPSocket.h"
#include "../socketexceptions/SocketException.hpp"

#include <cstring>

namespace kt
{
	TCPSocket::TCPSocket(const std::string& hostname, const unsigned int& port)
	{
		this->hostname = hostname;
		this->port = port;

		memset(&this->serverAddress, '\0', sizeof(this->serverAddress));

		constructWifiSocket();
	}

	TCPSocket::TCPSocket(const SOCKET& socket, const std::string& hostname, const unsigned int& port, const kt::InternetProtocolVersion protocolVersion, const kt::SocketAddress& acceptedAddress)
	{
		this->socketDescriptor = socket;
		this->hostname = hostname;
		this->port = port;
		this->protocolVersion = protocolVersion;
		std::memcpy(&this->serverAddress, &acceptedAddress, sizeof(this->serverAddress));
	}

	TCPSocket::TCPSocket(const kt::TCPSocket& socket)
	{
		this->socketDescriptor = socket.socketDescriptor;
		this->hostname = socket.hostname;
		this->port = socket.port;
		this->protocolVersion = socket.protocolVersion;
		std::memcpy(&this->serverAddress, &socket.serverAddress, sizeof(this->serverAddress));
	}

	TCPSocket& TCPSocket::operator=(const kt::TCPSocket& socket)
	{
		this->socketDescriptor = socket.socketDescriptor;
		this->hostname = socket.hostname;
		this->port = socket.port;
		this->protocolVersion = socket.protocolVersion;
		std::memcpy(&this->serverAddress, &socket.serverAddress, sizeof(this->serverAddress));

		return *this;
	}

	void TCPSocket::constructWifiSocket()
	{
		memset(&this->serverAddress, 0, sizeof(this->serverAddress));

#ifdef _WIN32
		WSADATA wsaData{};
		if (int res = WSAStartup(MAKEWORD(2, 2), &wsaData); res != 0)
		{
			throw kt::SocketException("WSAStartup Failed. " + std::to_string(res));
		}

#endif

		addrinfo* resolvedAddresses = nullptr;
		addrinfo hints{};
		hints.ai_family = static_cast<int>(this->protocolVersion);
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		int res = getaddrinfo(this->hostname.c_str(), std::to_string(this->port).c_str(), &hints, &resolvedAddresses);
		if (res != 0 || this->hostname.empty() || resolvedAddresses == nullptr)
		{
			if (resolvedAddresses != nullptr)
			{
				freeaddrinfo(resolvedAddresses);
			}

			throw kt::SocketException("Unable to resolve IP of destination address with hostname: [" + this->hostname + ":" + std::to_string(this->port) + "]. Look up response code: [" + std::to_string(res) + "]. " + getErrorCode());
		}

		// We need to iterate over the resolved address and attempt to connect to each of them, if a connection attempt is succesful 
		// we will return, otherwise we will throw is we are unable to connect to any.
		for (addrinfo* addr = resolvedAddresses; addr != nullptr; addr = addr->ai_next)
		{
			this->socketDescriptor = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
			if (!isInvalidSocket(this->socketDescriptor))
			{
				int connectionResult = connect(this->socketDescriptor, addr->ai_addr, addr->ai_addrlen);
				if (connectionResult == 0)
				{
					std::memcpy(&this->serverAddress, addr->ai_addr, addr->ai_addrlen);
					this->protocolVersion = static_cast<kt::InternetProtocolVersion>(addr->ai_family);
					freeaddrinfo(resolvedAddresses);
					// Return once we successfully connect to one
					return;
				}
			}

			this->close();
			this->socketDescriptor = getInvalidSocketValue();
		}
		freeaddrinfo(resolvedAddresses);
		throw kt::SocketException("Error connecting to TCP server: [" + std::to_string(res) + "] " + getErrorCode());
	}

	int TCPSocket::pollSocket(SOCKET socket, const long& timeout) const
	{
		timeval timeOutVal{};
		int res = kt::pollSocket(socket, timeout, &timeOutVal);

		return res;
	}

	void TCPSocket::close() const
	{
		kt::close(this->socketDescriptor);
	}

	bool TCPSocket::ready(const unsigned long timeout) const
	{
		int result = this->pollSocket(this->socketDescriptor, timeout);
		// 0 indicates that there is no data
		return result > 0;
	}

	bool TCPSocket::connected(const unsigned long timeout) const
	{
		int result = this->pollSocket(this->socketDescriptor, timeout);
		// -1 indicates that the connection is not available
		return result != -1;
	}

	bool TCPSocket::send(const char* message, const int& messageLength, const int& flags) const
	{
		int result = ::send(this->socketDescriptor, message, messageLength, flags);
		return result != -1;
	}

	bool TCPSocket::send(const std::string& message, const int& flags) const
	{
		return this->send(message.c_str(), message.size(), flags);
	}

	std::string TCPSocket::getHostname() const
	{
		return this->hostname;
	}

	unsigned int TCPSocket::getPort() const
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

	std::optional<char> TCPSocket::get(const int& flags) const
	{
		std::string received = this->receiveAmount(1, flags);
		if (received.empty())
		{
			return std::nullopt;
		}
		return received[0];
	}

	/**
	 * Reads in a specific amount of character from the input stream and returns them as a std::string.
	 * This method will return early if there is no more data to send or the other party closes the connection.
	 *
	 * @param amountToReceive - The amount of characters to read from the sender.
	 *
	 * @return A std::string of the specified size with the respective character read in.
	 */
	std::string kt::TCPSocket::receiveAmount(const unsigned int amountToReceive, const int& flags) const
	{
		std::string data;
		data.resize(amountToReceive);

		int amountReceived = this->receiveAmount(&data[0], amountToReceive, flags);
		return data.substr(0, amountReceived);
	}

	int TCPSocket::receiveAmount(char* buffer, const unsigned int amountToReceive, const int& flags) const
	{
		int counter = 0;

		if (amountToReceive == 0 || !this->ready())
		{
			return counter;
		}
		
		do
		{
			int amountReceived = recv(this->socketDescriptor, &buffer[counter], static_cast<int>(amountToReceive - counter), flags);
			if (amountReceived < 1)
			{
				return counter;
			}
			counter += amountReceived;
		} while (counter < amountToReceive && this->ready());

		return counter;
	}

	/**
	 * Reads from the sender until the passed in delimiter is reached. The delimiter is discarded and the characters preceeding it are returned as a std::string.
	 *
	 * @param delimiter The delimiter that will be used to mark the end of the read in process.
	 *
	 * @return A std::string with all of the characters preceeding the delimiter.
	 *
	 * @throw SocketException - if the delimiter is '\0'.
	 */
	std::string kt::TCPSocket::receiveToDelimiter(const char& delimiter, const int& flags)
	{
		if (delimiter == '\0')
		{
			throw kt::SocketException("The null terminator '\\0' is an invalid delimiter.");
		}

		std::string data;
		if (!this->ready())
		{
			return data;
		}

		std::optional<char> character;
		do
		{
			character = this->get(flags);
			if (character.has_value() && *character != delimiter)
			{
				data += *character;
			}
		} while (character.has_value() && *character != delimiter && this->ready());

		return data;
	}

	/**
	 * Reads data while the stream is *ready()*.
	 *
	 * @return A std::string containing the characters read while the stream was *ready()*.
	 *
	 * **NOTE:** This method can take a long time to execute. If you know the desired size and/or a delimiter, the other methods may be more fitting.
	 * This method may take some time for the receiver to retreive the whole message due to the inability to flush streams when using sockets.
	 */
	std::string kt::TCPSocket::receiveAll(const unsigned long timeout, const int& flags)
	{
		std::string result;
		result.reserve(1024);
		bool hitEOF = false;

		while (this->ready(timeout) && !hitEOF)
		{
			std::string res = receiveAmount(this->pollSocket(this->socketDescriptor, timeout), flags);
			if (!res.empty() && res[0] == '\0')
			{
				hitEOF = true;
			}
			else
			{
				result += res;
			}
		}
		result.shrink_to_fit();
		return result;
	}
}
