
#include "Socket.h"
#include "../socketexceptions/SocketException.hpp"
#include "../socketexceptions/BindingException.hpp"
#include "../enums/SocketProtocol.h"
#include "../enums/SocketType.h"
#include "../socketexceptions/SocketError.h"

#include <iostream>
#include <vector>
#include <utility>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <optional>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define _WIN32_WINNT 0x0600

#include <winsock2.h>
#include <winerror.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#include <guiddef.h>
#include <ws2bth.h>

#pragma comment(lib, "ws2_32.lib")

#elif __linux__

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif

namespace kt
{
	/**
	 * A constructor which will immediately attempt to connect to the host via the port specified.
	 * 
	 * @param hostname - The hostname of the device to connect to.
	 * @param port - The port number.
	 * @param type - Determines whether this socket is a wifi or bluetooth socket. 
	 * @param protocol - Indicates the protocol being used by this socket, for Wifi this value can be *kt::SocketProtocol::TCP* or *kt::SocketProtocol::UDP* Default value is *kt::SocketProtocol::None*.
	 * 
	 * @throw SocketException - If the Socket is unable to be instanciated or connect to server.
	 * @throw BindingException - If the Socket is unable to bind to the port specified.
	 */
	kt::Socket::Socket(const std::string& hostname, const unsigned int& port, const kt::SocketType type, const kt::SocketProtocol protocol)
	{
		this->hostname = hostname;
		this->port = port;
		this->type = type;
		this->protocol = protocol;

		memset(&this->receiveAddress, '\0', sizeof(this->receiveAddress));
		memset(&this->serverAddress, '\0', sizeof(this->serverAddress));

#ifdef __linux__
		this->bluetoothAddress = { 0 };

#endif

		if (this->type == kt::SocketType::Wifi && this->protocol == kt::SocketProtocol::None)
        {
            throw SocketException("Unable to set protocol to 'None' for a Wifi Socket.");
        }
		else if (this->type == kt::SocketType::Bluetooth && this->protocol != kt::SocketProtocol::None)
		{
			// No protocol should be set when using a bluetooth socket
			throw kt::SocketException("Bluetooth socket Protocol should be 'None'.");
		}

	    if (type == kt::SocketType::Wifi)
	    {
			this->constructWifiSocket(this->port);
	    }
	    else if (type == kt::SocketType::Bluetooth)
	    {
			this->constructBluetoothSocket();
	    }
		else
		{
			// kt::SocketType::None
			throw kt::SocketException("Unable to build Socket with 'None' as its SocketType");
		}
	}

	/**
	 * A constructor used by ServerSocket to create and copy of a currently connected socket. **This should not be used directly**.
	 * 
	 * @param socketDescriptor - Is the file descriptor for the connection.
	 * @param type - Determines whether this socket is a wifi or bluetooth socket.
	 * @param protocol - Indicates the protocol being used by this socket, for Wifi this value can be *kt::SocketProtocol::TCP* or *kt::SocketProtocol::UDP* Default value is *kt::SocketProtocol::None*.
	 * @param hostname - the hostname of the socket to copy.
	 * @param port - the port number of the socket to copy.
	 * @param protocolVersion - the protocol version that the socket will use.
	 */
	kt::Socket::Socket(const SOCKET& socketDescriptor, const kt::SocketType type, const kt::SocketProtocol protocol, const std::string& hostname, const unsigned int& port, const kt::InternetProtocolVersion protocolVersion)
	{
		this->hostname = hostname;
		this->port = port;
		this->protocol = protocol;
		this->socketDescriptor = socketDescriptor;
		this->type = type;
		this->protocolVersion = protocolVersion;
		memset(&this->receiveAddress, '\0', sizeof(this->receiveAddress));
		memset(&this->serverAddress, '\0', sizeof(this->serverAddress));
	}

	/**
	 * A copy constructor for the Socket class. Will copy the object members and assume that it is already connected to the endpoint.
	 * 
	 * @param socket - The Socket object to be copied.
	 */
	kt::Socket::Socket(const kt::Socket& socket)
	{
		this->socketDescriptor = socket.socketDescriptor;
		this->udpSendSocket = socket.udpSendSocket;
		this->hostname = socket.hostname;
		this->protocol = socket.protocol;
		this->port = socket.port;
		this->type = socket.type;
		this->protocolVersion = socket.protocolVersion;

		this->receiveAddress = socket.receiveAddress;
		this->serverAddress = socket.serverAddress;
#ifdef __linux__
		this->bluetoothAddress = socket.bluetoothAddress;

#endif
		
	}

	/**
	 * An assignment operator for the Socket object. Will make a copy of the appropriate socket.
	 * 
	 * @param socket - The Socket object to be copied.
	 * 
	 * @return kt::Socket the copied socket
	 */
	kt::Socket& kt::Socket::operator=(const kt::Socket& socket)
	{
		this->socketDescriptor = socket.socketDescriptor;
		this->udpSendSocket = socket.udpSendSocket;
		this->hostname = socket.hostname;
		this->protocol = socket.protocol;
		this->port = socket.port;
		this->type = socket.type;
		this->protocolVersion = socket.protocolVersion;

		this->receiveAddress = socket.receiveAddress;
		this->serverAddress = socket.serverAddress;
#ifdef __linux__
		this->bluetoothAddress = socket.bluetoothAddress;

#endif

		return *this;
	}

	void kt::Socket::constructBluetoothSocket()
	{
#ifdef _WIN32
		throw kt::SocketException("Socket:constructBluetoothSocket() is not supported on Windows.");

		/*this->socketDescriptor = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
		if (isInvalidSocket(this->socketDescriptor))
		{
			throw SocketException("Error establishing Bluetooth socket: " + std::string(std::strerror(errno)));
		}

		this->bluetoothAddress.addressFamily = AF_BTH;
		this->bluetoothAddress.btAddr = std::stoull(this->hostname);
		this->bluetoothAddress.port = this->port;

		if (connect(this->socketDescriptor, (sockaddr*)&this->bluetoothAddress, sizeof(SOCKADDR_BTH)) == -1)
		{
			throw SocketException("Error connecting to Bluetooth server: " + std::string(std::strerror(errno)));
		}*/

#elif __linux__
		this->socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

		if (isInvalidSocket(this->socketDescriptor))
	    {
	    	throw SocketException("Error establishing Bluetooth socket: " + std::string(std::strerror(errno)));
	    }

	    this->bluetoothAddress.rc_family = AF_BLUETOOTH;
	    this->bluetoothAddress.rc_channel = (uint8_t) port;
	    str2ba(this->hostname.c_str(), &this->bluetoothAddress.rc_bdaddr);

	   	if (connect(this->socketDescriptor, (sockaddr*) &this->bluetoothAddress, sizeof(this->bluetoothAddress)) == -1)
	   	{
	   		throw SocketException("Error connecting to Bluetooth server: " + std::string(std::strerror(errno)));
		}
#endif
	}

	void kt::Socket::constructWifiSocket(unsigned int& newPort)
	{
		const int socketType = this->protocol == kt::SocketProtocol::TCP ? SOCK_STREAM : SOCK_DGRAM;
		const int socketProtocol = this->protocol == kt::SocketProtocol::TCP ? IPPROTO_TCP : IPPROTO_UDP;

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
		hints.ai_socktype = socketType;
		hints.ai_protocol = socketProtocol;
		int res = getaddrinfo(this->hostname.c_str(), std::to_string(newPort).c_str(), &hints, &resolvedAddresses);
		if (res != 0 || this->hostname.empty() || resolvedAddresses == nullptr)
		{
			if (resolvedAddresses != nullptr)
			{
				freeaddrinfo(resolvedAddresses);
			}

			throw kt::SocketException("Unable to resolve IP of destination address with hostname: [" + this->hostname + "]. Look up response code: [" + std::to_string(res) + "]. " + getErrorCode());
		}

		// We need to iterate over the resolved address and attempt to connect to each of them, if a connection attempt is succesful 
		// we will return, otherwise we will throw is we are unable to connect to any.
		if (this->protocol == kt::SocketProtocol::TCP)
		{
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
			throw kt::SocketException("Error connecting to Wifi server: [" + std::to_string(res) + "] " + getErrorCode());
		}
		else
		{
			this->udpSendSocket = socket(resolvedAddresses->ai_family, resolvedAddresses->ai_socktype, resolvedAddresses->ai_protocol);
			this->protocolVersion = static_cast<kt::InternetProtocolVersion>(resolvedAddresses->ai_family);
			std::memcpy(&this->serverAddress, resolvedAddresses->ai_addr, resolvedAddresses->ai_addrlen);
			freeaddrinfo(resolvedAddresses);
		}
	}

	/**
	 * Closes the existing connection. If no connection is open, then it will do nothing.
	 * This method should be called before the object itself is distructed.
	 */
	void kt::Socket::close()
	{
		this->close(this->socketDescriptor);
		this->close(this->udpSendSocket);

		this->bound = false;
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
	bool kt::Socket::bind(const kt::InternetProtocolVersion protocolVersion)
	{
		if (this->protocol == kt::SocketProtocol::UDP)
		{
			// Clear client address
			memset(&this->receiveAddress, '\0', sizeof(this->receiveAddress));
			
			const int socketType = SOCK_DGRAM;
			const int socketProtocol = IPPROTO_UDP;
			
			addrinfo hint{};
			hint.ai_flags = AI_PASSIVE;
			hint.ai_family = static_cast<int>(protocolVersion);
			hint.ai_socktype = socketType;
			hint.ai_protocol = socketProtocol;

			addrinfo *addresses = nullptr;
			if (getaddrinfo(nullptr, std::to_string(this->port).c_str(), &hint, &addresses) != 0)
			{
				freeaddrinfo(addresses);
				throw kt::SocketException("Failed to retrieve address info of local host. " + getErrorCode());
			}
			this->protocolVersion = static_cast<kt::InternetProtocolVersion>(addresses->ai_family);
			this->socketDescriptor = socket(addresses->ai_family, addresses->ai_socktype, addresses->ai_protocol);

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

			this->bound = ::bind(this->socketDescriptor, addresses->ai_addr, addresses->ai_addrlen) != -1;
			freeaddrinfo(addresses);
			if (!this->bound)
			{
				throw kt::BindingException("Error binding connection, the port " + std::to_string(this->port) + " is already being used: " + getErrorCode());
			}

			if (this->port == 0)
			{
				this->initialiseListeningPortNumber();
			}

			return this->bound;
		}
		return false;
	}

	void kt::Socket::initialiseListeningPortNumber()
    {
        socklen_t socketSize = sizeof(this->serverAddress);
        if (getsockname(this->socketDescriptor, &this->serverAddress.address, &socketSize) != 0)
        {
            this->close();
            throw kt::BindingException("Unable to retrieve randomly bound port number during socket creation. " + getErrorCode());
        }

        if (this->protocolVersion == kt::InternetProtocolVersion::IPV6)
        {
            this->port = ntohs(this->serverAddress.ipv6.sin6_port);
        }
        else
        {
            this->port = ntohs(this->serverAddress.ipv4.sin_port);
        }
    }

	void Socket::close(SOCKET socket)
	{
#ifdef _WIN32
		closesocket(socket);

#elif __linux__
		::close(socket);
#endif
	}

	/**
	 * Sends input std::string to the receiver via the configured socket.
	 * 
	 * @param message - The message to send to the receiver.
	 * @param flag - A flag value to specify additional behaviour for this message. *Defaults to 0 if no argument is passed*.
	 * 
	 * @return true if the message was sent without error, else false.
	 */
	bool kt::Socket::send(const std::string& message, int flag)
	{
		if (!message.empty())
		{
			if (this->protocol == kt::SocketProtocol::TCP)
			{
				return ::send(this->socketDescriptor, message.c_str(), message.size(), flag) != -1;
			}
			else if (this->protocol == kt::SocketProtocol::UDP)
			{
				std::optional<kt::SocketAddress> address = this->getUDPSendAddress();
				if (address.has_value())
				{
					int result = ::sendto(this->udpSendSocket, message.c_str(), message.size(), flag, &(address.value().address), sizeof(address.value()));
					return result != -1;
				}
				return false;
			}
		}
		return false;
	}

	int kt::Socket::pollSocket(SOCKET socket, const long& timeout) const
	{
		timeval timeOutVal{};
		int res = kt::pollSocket(socket, timeout, &timeOutVal);
#ifdef __linux__
		if (res == 0)
		{
			if (timeOutVal.tv_usec == 0)
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
#endif

		return res;
	}

	/**
	 * Poll the provided socket descriptor for the provided timeout in microseconds.
	 */
	int pollSocket(const SOCKET& socketDescriptor, const long& timeout, timeval* timeOutVal)
	{
		fd_set sReady{};
		timeval timeoutVal{};
		if (timeOutVal == nullptr)
		{
			timeOutVal = &timeoutVal;
		}
		
		timeOutVal->tv_usec = static_cast<long>(timeout);

		FD_ZERO(&sReady);
		FD_SET(socketDescriptor, &sReady);

		// Need this->socketDescriptor + 1 here
		return select(socketDescriptor + 1, &sReady, nullptr, nullptr, timeOutVal);
	}

	/**
	 * Determines whether the stream has data to read.
	 * 
	 * @param timeout - The timeout duration in *micro seconds*. Default is 1000 microseconds.
	 * 
	 * @return true if there is data to read otherwise false.
	 */
	bool kt::Socket::ready(const unsigned long timeout) const
	{
		int result = this->pollSocket(this->socketDescriptor, timeout);
		// 0 indicates that there is no data
		return result > 0;
	}

	/**
	 * Determines whether the stream is open.
	 * 
	 * @param timeout - The timeout duration in *micro seconds*. Default is 1000 microseconds.
	 * 
	 * @return true if the stream is open otherwise false.
	 * 
	 * **NOTE:** This method is still in BETA, and cannot detect if the connection has been closed by the remote device.
	 */
	bool kt::Socket::connected(const unsigned long timeout) const
	{
		// UDP is connectionless
		if (this->protocol == kt::SocketProtocol::UDP)
		{
			return false;
		}

		int result = this->pollSocket(this->socketDescriptor, timeout);

		// -1 indicates that the connection is not available
		return result != -1;
	}

	/**
	 * This method is intended only for kt::SocketProtcol::UDP kt::Sockets.
	 * Usage on a *kt::SocketProtocol::TCP* socket will always return *false*.
	 * 
	 * @return true if this socket is bound, otherwise false. 
	 */
	bool kt::Socket::isUdpBound() const
	{
		return this->bound;
	}

	/**
	 * Reads and returns a single character from the reciever.
	 * 
	 * @return The character read.
	 */
	std::optional<char> kt::Socket::get()
	{
		std::string received = this->receiveAmount(1);
		if (received.empty())
		{
			return std::nullopt;
		}
		return received[0];
	}

	/**
	 * @return the port number used by this socket.
	 */
	unsigned int kt::Socket::getPort() const
	{
		return this->port;
	}

	/**
	 * @return the kt::SocketType for this kt::Socket.
	 */
	kt::SocketType kt::Socket::getType() const
	{
		return this->type;
	}

	/**
	 * @return the kt::InternetProtocolVersion for this kt::Socket.
	*/
	kt::InternetProtocolVersion kt::Socket::getInternetProtocolVersion() const
    {
        return this->protocolVersion;
    }

    /**
	 * @return the kt::SocketProtocol configured for this kt::Socket.
	 */
	kt::SocketProtocol kt::Socket::getProtocol() const
	{
		return this->protocol;
	}

	/**
	 * This method is intended only for kt::SocketProtcol::UDP kt::Sockets.
	 * Use on a kt::SocketProtcol::TCP will result in an empty string.
	 * 
	 * @return when using *kt::SocketProtocol::UDP* the address of the last device who sent the data that was most recently read. Always returns an empty string for kt::SocketProtocol::TCP kt::Sockets.
	 */
	std::optional<std::string> kt::Socket::getLastUDPRecievedAddress() const
	{
		if (this->protocol == kt::SocketProtocol::UDP)
		{
			return kt::resolveToAddress(&this->receiveAddress, this->getInternetProtocolVersion());
		}

		return std::nullopt;
	}

	std::optional<kt::SocketAddress> kt::Socket::getLastUDPReceivedAddress() const
	{
		if (this->protocol == kt::SocketProtocol::UDP)
		{
			kt::SocketAddress address{};
			std::memcpy(&address, &this->receiveAddress, sizeof(address));
			return std::optional{ address };
		}
		return std::nullopt;
	}

	std::optional<std::string> resolveToAddress(const kt::SocketAddress* address, const kt::InternetProtocolVersion protocolVersion)
	{
		const size_t addressLength = protocolVersion == kt::InternetProtocolVersion::IPV6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN;
		std::string asString;
		asString.resize(addressLength);

		if (protocolVersion == kt::InternetProtocolVersion::IPV6)
		{
			inet_ntop(static_cast<int>(protocolVersion), &address->ipv6.sin6_addr, &asString[0], addressLength);
		}
		else
		{
			inet_ntop(static_cast<int>(protocolVersion), &address->ipv4.sin_addr, &asString[0], addressLength);
		}

		// Removing trailing \0 bytes
		const size_t delimiterIndex = asString.find_first_of('\0');
		if (delimiterIndex != std::string::npos) 
		{
			asString = asString.substr(0, delimiterIndex);
		}
		// Since we zero out the address, we need to check its not default initialised
		return !asString.empty() && asString != "0.0.0.0" && asString != "::" ? std::optional<std::string>{asString} : std::nullopt;
	}

	/**
	 * @return the hostname configured for this socket.
	 */
	std::string kt::Socket::getHostname() const
	{
		return this->hostname;
	}

	std::optional<kt::SocketAddress> kt::Socket::getUDPSendAddress() const
	{
		if (this->protocol == kt::SocketProtocol::UDP)
		{
			kt::SocketAddress newAddress{};
			// If the current server address is empty then we should return a null optional
			if (std::memcmp(&newAddress, &this->serverAddress, sizeof(newAddress)) == 0)
			{
				return std::nullopt;
			}
			
			memcpy(&newAddress, &this->serverAddress, sizeof(this->serverAddress));
			return std::optional{ newAddress };
		}
		return std::nullopt;
	}

	void Socket::setUDPSendAddress(std::string newHostname, unsigned int newPort, kt::InternetProtocolVersion newProtocolVersion)
	{
		this->protocolVersion = newProtocolVersion;
		this->hostname = newHostname;
		this->constructWifiSocket(newPort);
	}

	void Socket::setUDPSendAddress(kt::SocketAddress newSocketAddress)
	{
		if (this->getProtocol() == kt::SocketProtocol::UDP)
		{
			kt::InternetProtocolVersion newVersion = kt::getInternetProtocolVersion(newSocketAddress);
			if (newVersion != this->getInternetProtocolVersion())
			{
				this->close(this->udpSendSocket);
				this->udpSendSocket = socket(newSocketAddress.address.sa_family, SOCK_DGRAM, IPPROTO_UDP);
				this->protocolVersion = newVersion;
			}
			std::memcpy(&this->serverAddress, &newSocketAddress, sizeof(this->serverAddress));
		}
	}

	/**
	 * Reads in a specific amount of character from the input stream and returns them as a std::string.
	 * This method will return early if there is no more data to send or the other party closes the connection.
	 * 
	 * @param amountToReceive - The amount of characters to read from the sender.
	 * 
	 * @return A std::string of the specified size with the respective character read in. 
	 */
	std::string kt::Socket::receiveAmount(const unsigned int amountToReceive)
	{
		if (amountToReceive == 0 || !this->ready())
		{
			return "";
		}

		std::string data;
		data.resize(amountToReceive);

		std::string result;
		result.reserve(amountToReceive);

		if (this->protocol == kt::SocketProtocol::TCP)
		{
			unsigned int counter = 0;

			do
			{
				int flag = recv(this->socketDescriptor, &data[0], static_cast<int>(amountToReceive - counter), 0);
				if (flag < 1)
				{
					return result;
				}
				
				// Need to substring to remove null terminating byte
				result += data.substr(0, flag);
				
				data.clear();
				counter += flag;
			} while (counter < amountToReceive && this->ready());
		}
		else if (this->protocol == kt::SocketProtocol::UDP)
		{
			// UDP is odd, and will consume the entire datagram after a single read even if not all bytes are read
			memset(&this->receiveAddress, '\0', sizeof(this->receiveAddress));
			socklen_t addressLength = sizeof(this->receiveAddress);
			int flag = recvfrom(this->socketDescriptor, &data[0], static_cast<int>(amountToReceive), 0, &this->receiveAddress.address, &addressLength);
			if (flag < 1)
			{
				// This is for Windows, in some scenarios Windows will return a -1 flag but the buffer is populated properly
				// The code it is returning is 10040 this is indicating that the provided buffer is too small for the incoming
				// message, there is probably some settings we can tweak, however I think this is okay to return for now.
				return data;
			}

			// Need to substring to remove null terminating byte
			result += data.substr(0, flag);
		}
		return result;
	}

	/**
	 * Reads from the sender until the passed in delimiter is reached. The delimiter is discarded and the characters preceeding it are returned as a std::string.
	 * 
	 * @param delimiter The delimiter that will be used to mark the end of the read in process.
	 * @param udpMaxAmountToRead Only used when using UDP, it is set as a packet read limit, which the delimiter is scanned for then the remaining data after it is truncated and lost.
	 * 
	 * @return A std::string with all of the characters preceeding the delimiter.
	 * 
	 * @throw SocketException - if the delimiter is '\0'.
	 */
	std::string kt::Socket::receiveToDelimiter(const char& delimiter, unsigned int udpMaxAmountToRead)
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

		if (this->protocol == kt::SocketProtocol::TCP)
		{
			std::optional<char> character;
			do
			{
				character = this->get();
				if (character.has_value() && *character != delimiter)
				{
					data += *character;
				}
			} while (character.has_value() && *character != delimiter && this->ready());

			return data;
		}
		else if (this->protocol == kt::SocketProtocol::UDP)
		{
			std::string temp;
			if (udpMaxAmountToRead == 0)
			{
				udpMaxAmountToRead = DEFAULT_UDP_BUFFER_SIZE;
			}
			temp.resize(udpMaxAmountToRead);
			memset(&this->receiveAddress, '\0', sizeof(this->receiveAddress));
			socklen_t addressLength = sizeof(this->receiveAddress);
			
			int flag = recvfrom(this->socketDescriptor, &temp[0], static_cast<int>(udpMaxAmountToRead), 0, &this->receiveAddress.address, &addressLength);
			if (flag < 1)
			{
				return data;
			}

			// Need to substring to remove null terminating byte
			data += temp.substr(0, flag);
			size_t delimiterIndex = data.find_first_of(delimiter);
			if (delimiterIndex != std::string::npos)
			{
				return data.substr(0, delimiterIndex);
			}
		}

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
	std::string kt::Socket::receiveAll(const unsigned long timeout)
	{
		if (this->protocol == kt::SocketProtocol::UDP)
		{
			throw kt::SocketException("Socket::receiveAll(unsigned long) is not supported for UDP socket configuration. Please use Socket::receiveAmount(unsigned int) instead.");
		}

		std::string result;
		result.reserve(1024);
		bool hitEOF = false;

		if (!this->ready())
		{
			return "";
		}

		while (this->ready(timeout) && !hitEOF)
		{
			std::string res = receiveAmount(this->pollSocket(this->socketDescriptor, timeout));
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

	/**
	 * **In progress**
	 * 
	 * Scans for bluetooth devices and returns a std::vector&lt;std::pair&lt;std::string, std::string>> of the device names and addresses.
	 * 
	 * @param duration - The duration for which the scan should take to discover nearby bluetooth devices.
	 * 
	 * @return A std::vector&lt;std::pair&lt;std::string, std::string>> where .first is the devices address, and .second is the device name.
	 */
	std::vector<std::pair<std::string, std::string> > kt::Socket::scanDevices(unsigned int duration)
	{
#ifdef _WIN32
		throw kt::SocketException("Socket::scanDevices(int) is not supported on Windows.");

		/*WSADATA wsaData;
		int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (res != 0)
		{
			throw SocketException("WSAStartup Failed: " + std::to_string(res));
		}*/

		/*WSAQUERYSET wsaQuery;
		HANDLE hLoopUp;
		LPWSAQUERYSET pQuerySet = nullptr;
		SOCKADDR_BTH tempAddress;
		DWORD dwSize = 5000 * sizeof(unsigned char);
		memset(&wsaQuery, 0, sizeof(WSAQUERYSET));
		wsaQuery.dwSize = sizeof(WSAQUERYSET);
		wsaQuery.dwNameSpace = NS_BTH;
		wsaQuery.lpcsaBuffer = nullptr;

		int res = WSALookupServiceBegin(&wsaQuery, LUP_CONTAINERS, &hLoopUp);
		if (res == -1)
		{
			throw SocketException("Unable to search for devices. Could not begin search.");
		}

		memset(&pQuerySet, 0, sizeof(WSAQUERYSET));
		pQuerySet->dwSize = sizeof(WSAQUERYSET);
		pQuerySet->dwNameSpace = NS_BTH;
		pQuerySet->lpBlob = nullptr;

		while (WSALookupServiceNext(hLoopUp, LUP_RETURN_NAME | LUP_RETURN_ADDR, &dwSize, pQuerySet) == 0)
		{
			tempAddress = ((SOCKADDR_BTH*) pQuerySet->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr;
			// std::cout << pQuerySet->lpszServiceInstanceName << " : " << GET_NAP(tempAddress) << " - " << GET_SAP(tempAddress) << " ~ " << pQuerySet->dwNameSpace << std::endl;
		}*/

#elif __linux__

		std::vector<std::pair<std::string, std::string> > devices;
		std::pair<std::string, std::string> tempPair;

		inquiry_info *ii = nullptr;
	    int maxResponse = 255, numberOfResponses, ownId, tempSocket, flags;
	    char deviceAddress[19];
	    char deviceName[248];

	    ownId = hci_get_route(nullptr);
	    tempSocket = hci_open_dev( ownId );
	    if (ownId < 0 || tempSocket < 0)
	    {
	        throw SocketException("Error opening Bluetooth socket for scanning...");
	    }

	    flags = IREQ_CACHE_FLUSH;
	    ii = new inquiry_info[maxResponse * sizeof(inquiry_info)];
	    
	    numberOfResponses = hci_inquiry(ownId, duration, maxResponse, nullptr, &ii, flags);
	    if( numberOfResponses < 0 )
	    {
	    	delete []ii;
	    	throw SocketException("Error scanning for bluetooth devices");
	    }

	    for (int i = 0; i < numberOfResponses; i++) 
	    {
	        ba2str( &(ii + i)->bdaddr, deviceAddress);
			memset(&deviceName, '\0', sizeof(deviceName));
	        if (hci_read_remote_name(tempSocket, &(ii+i)->bdaddr, sizeof(deviceName), deviceName, 0) < 0)
	        {
	        	strcpy(deviceName, "[unknown]");
	        }

	        tempPair = std::make_pair<std::string, std::string>(deviceAddress, deviceName);
	        devices.push_back(tempPair);

	    }

	    delete []ii;
	    ::close( tempSocket );

		return devices;
#endif
	}

	std::optional<std::string> kt::Socket::getLocalMACAddress()
	{
#ifdef _WIN32
		throw kt::SocketException("Socket::getLocalMACAddress() is not supported on Windows.");

		// Up to 20 Interfaces
		/*IP_ADAPTER_INFO AdapterInfo[20];
		DWORD dwBufLen = sizeof(AdapterInfo);
		DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;

		while (std::string(pAdapterInfo->Description).find("Bluetooth") == std::string::npos)
		{
			pAdapterInfo = pAdapterInfo->Next;
		}

		std::stringstream ss;
		for (int i = 0; i < 6; i++)
		{
			ss << std::hex << std::setfill('0');
			ss << std::setw(2) << static_cast<unsigned>(pAdapterInfo->Address[i]);

			if (i != 5)
			{
				ss << ":";
			}
		}

		return ss.str();*/

#elif __linux__
		int id;
		bdaddr_t btaddr;
		char localMACAddress[18];
	
		// Get id of local device
		if ((id = hci_get_route(nullptr)) < 0)
		{
			return std::nullopt;
		}
	
		// Get local bluetooth address
		if (hci_devba(id, &btaddr) < 0)
		{
			return std::nullopt;
		}
	
		// Convert address to string
		if (ba2str(&btaddr, localMACAddress) < 0)
		{
			return std::nullopt;
		}
		
		return std::optional<std::string>{std::string(localMACAddress)};
#endif
	}

} // End namespace kt
