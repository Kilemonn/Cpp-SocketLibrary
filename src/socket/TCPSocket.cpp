
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

		constructWifiSocket();
	}

	TCPSocket::TCPSocket(const SOCKET& socket, const std::string& hostname, const unsigned short& port, const kt::InternetProtocolVersion protocolVersion, const kt::SocketAddress& acceptedAddress)
	{
		this->socketDescriptor = socket;
		this->hostname = hostname;
		this->port = port;
		this->protocolVersion = protocolVersion;
		this->serverAddress = acceptedAddress;
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

	void TCPSocket::constructWifiSocket()
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
			this->socketDescriptor = getInvalidSocketValue();
		}
		
		throw kt::SocketException("Unable to connect to resolved addresses for provided hostname [" + this->hostname + ":" + std::to_string(this->port) + "] " + getErrorCode());
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

	std::pair<bool, int> TCPSocket::send(const char* message, const int& messageLength, const int& flags) const
	{
		int result = ::send(this->socketDescriptor, message, messageLength, flags);
		return std::make_pair(result != -1, result);
	}

	std::pair<bool, int> TCPSocket::send(const std::string& message, const int& flags) const
	{
		return this->send(message.c_str(), message.size(), flags);
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
			int amountReceived = ::recv(this->socketDescriptor, &buffer[counter], static_cast<int>(amountToReceive - counter), flags);
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
