
#include "Socket.h"
#include "../socketexceptions/SocketException.hpp"
#include "../socketexceptions/BindingException.hpp"
#include "../enums/SocketProtocol.cpp"
#include "../enums/SocketType.cpp"
#include "../template/SocketSerialiser.h"

#include <iostream>
#include <vector>
#include <utility>
#include <cstring>
#include <sstream>
#include <iomanip>

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
	 * @throw SocketException - If the Socket is unable to be instantiated or connect to server.
	 * @throw BindingException - If the Socket is unable to bindUdpSocket to the port specified.
	 */
	Socket::Socket(const std::string& hostname, const unsigned int& port, const kt::SocketType type, const kt::SocketProtocol protocol)
	{
		this->hostname = hostname;
		this->port = port;
		this->type = type;
		this->protocol = protocol;
		this->serverAddress = { 0 };
		this->socketDescriptor = 0;
		this->bluetoothAddress = { 0 };

		if (this->type == kt::SocketType::Wifi && this->protocol == kt::SocketProtocol::None)
        {
            throw SocketException("Unable to set protocol to 'None' for a Wifi Socket.");
        }
		else if (this->type == kt::SocketType::Bluetooth && this->protocol != kt::SocketProtocol::None)
		{
			// No protocol should be set when using a bluetooth socket
			throw SocketException("Bluetooth socket Protocol should be 'None'.");
		}

        if (type == kt::SocketType::Wifi)
        {
            this->constructWifiSocket();
        }
        else if (type == kt::SocketType::Bluetooth)
        {
            this->constructBluetoothSocket();
        }
        else
        {
            // kt::SocketType::None
            throw SocketException("Unable to build Socket with 'None' as its SocketType");
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
	 */
	Socket::Socket(const int& socketDescriptor, const kt::SocketType type, const kt::SocketProtocol protocol, const std::string& hostname, const unsigned int& port)
	{
		this->hostname = hostname;
		this->port = port;
		this->protocol = protocol;
		this->socketDescriptor = socketDescriptor;
		this->type = type;
	}

	/**
	 * A copy constructor for the Socket class. Will copy the object members and assume that it is already connected to the endpoint.
	 * 
	 * @param socket - The Socket object to be copied.
	 */
	Socket::Socket(const Socket& socket)
	{
		this->socketDescriptor = socket.socketDescriptor;
		this->hostname = socket.hostname;
		this->protocol = socket.protocol;
		this->port = socket.port;
		this->type = socket.type;

		this->serverAddress = socket.serverAddress;
		this->bluetoothAddress = socket.bluetoothAddress;
	}

	/**
	 * An assignment operator for the Socket object. Will make a copy of the appropriate socket.
	 * 
	 * @param socket - The Socket object to be copied.
	 * 
	 * @return kt::Socket the copied socket
	 */
	Socket& Socket::operator=(const Socket& socket)
	{
		this->socketDescriptor = socket.socketDescriptor;
		this->hostname = socket.hostname;
		this->protocol = socket.protocol;
		this->port = socket.port;
		this->type = socket.type;

		this->serverAddress = socket.serverAddress;
		this->bluetoothAddress = socket.bluetoothAddress;

		return *this;
	}

	void Socket::constructBluetoothSocket()
	{
		this->socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

		if (this->socketDescriptor == -1)
	    {
	    	throw SocketException("Error establishing Bluetooth socket: " + std::string(std::strerror(errno)));
	    }

	    this->bluetoothAddress.rc_family = AF_BLUETOOTH;
	    this->bluetoothAddress.rc_channel = (uint8_t) port;
	    str2ba(this->hostname.c_str(), &this->bluetoothAddress.rc_bdaddr);

	   	if (connect(this->socketDescriptor, (struct sockaddr *) &this->bluetoothAddress, sizeof(this->bluetoothAddress)) == -1)
	   	{
	   		throw SocketException("Error connecting to Bluetooth server: " + std::string(std::strerror(errno)));
		}
	}

	void Socket::constructWifiSocket()
	{
		int socketProtocol = this->protocol == kt::SocketProtocol::TCP ? SOCK_STREAM : SOCK_DGRAM;
	    this->socketDescriptor = socket(AF_INET, socketProtocol, 0);

	    if (this->socketDescriptor == -1)
	    {
	    	throw SocketException("Error establishing Wifi socket: " + std::string(std::strerror(errno)));
	    }

		struct hostent* server = gethostbyname(this->hostname.c_str());
		if (server != nullptr)
		{
			memset(&this->serverAddress, 0, sizeof(this->serverAddress));
			this->serverAddress.sin_family = AF_INET;
			bcopy((char *) server->h_addr, (char *) &this->serverAddress.sin_addr.s_addr, server->h_length);
			this->serverAddress.sin_port = htons(this->port);
		}
		else
		{
			throw SocketException("Unable to resolve IP of destination address with hostname: " + this->hostname + ".");
		}

		if (this->protocol == kt::SocketProtocol::TCP)
		{
			if (connect(this->socketDescriptor, (struct sockaddr *)&this->serverAddress, sizeof(this->serverAddress)) == -1)
			{
				throw SocketException("Error connecting to Wifi server: " + std::string(std::strerror(errno)));
			}
		}
	}

	/**
	 * Closes the existing connection. If no connection is open, then it will do nothing.
	 * This method should be called before the object itself is distructed.
	 */
	void Socket::close()
	{
		::close(this->socketDescriptor);
		this->bound = false;
	}

	/**
	 * This method is required for kt::SocketProtocol::UDP sockets. 
	 * The socket that is listening for new connections will need to call this before they begin listening (accepting connections). 
	 * This ensures the socket is bound to the port and can receive new connections. *Only a single process can be bound to a single port at one time*.
	 * 
	 * @return bool - true if the socket was bound successfully, otherwise false
	 * 
	 * @throw BindingException - if the socket fails to bindUdpSocket
	 */ 
	bool Socket::bindUdpSocket()
	{
		if (this->protocol == kt::SocketProtocol::UDP)
		{
			// Clear client address
			memset(&this->clientAddress, 0, sizeof(this->clientAddress));

			struct sockaddr_in localAddress;
			localAddress.sin_family = AF_INET;
			localAddress.sin_port = htons(this->port);
			localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
			this->bound = ::bind(this->socketDescriptor, (struct sockaddr*) &localAddress, sizeof(localAddress)) != -1;
			if (!this->bound)
			{
				throw BindingException("Error binding connection, the port " + std::to_string(this->port) + " is already being used: " + std::string(std::strerror(errno)));
			}
			return this->bound;
		}
		return false;
	}

	/**
	 * Sends input std::string to the receiver via the configured socket.
	 * 
	 * @param message - The message to send to the receiver.
	 * @param flag - A flag value to specify additional behaviour for this message. *Defaults to 0 if no argument is passed*.
	 * 
	 * @return true if the message was sent without error, else false.
	 */
	bool Socket::send(const std::string& message, int flag)
	{
		if (!message.empty())
		{
            return this->send(message.c_str(), message.size(), flag);
		}
		return false;
	}

	bool Socket::send(const char* bytes, unsigned int size, int flag)
    {
        if (this->protocol == kt::SocketProtocol::TCP)
        {
            return ::send(this->socketDescriptor, bytes, size, flag) != -1;
        }
        else if (this->protocol == kt::SocketProtocol::UDP)
        {
            struct sockaddr_in address = this->getSendAddress();
            return ::sendto(this->socketDescriptor, bytes, size, flag, (const struct sockaddr *)&address, sizeof(address)) != -1;
        }
        return false;
    }

	int Socket::pollSocket(const unsigned long timeout) const
	{
		fd_set sReady;
		struct timeval timeOutVal;

		memset((char*) &timeOutVal, 0, sizeof(timeOutVal));
		timeOutVal.tv_usec = static_cast<long>(timeout);

		FD_ZERO(&sReady);
		FD_SET(this->socketDescriptor, &sReady);

		// Need this->socketDescriptor + 1 here
		int res = select(this->socketDescriptor + 1, &sReady, nullptr, nullptr, &timeOutVal);
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
		return res;
	}

	/**
	 * Determines whether the stream has data to read.
	 * 
	 * @param timeout - The timeout duration in *micro seconds*. Default is 1000 microseconds.
	 * 
	 * @return true if there is data to read otherwise false.
	 */
	bool Socket::ready(const unsigned long timeout) const
	{
		int result = this->pollSocket(timeout);
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
	bool Socket::connected(const unsigned long timeout) const
	{
		// UDP is connectionless
		if (this->protocol == kt::SocketProtocol::UDP)
		{
			return false;
		}

		int result = this->pollSocket(timeout);

		// -1 indicates that the connection is not available
		return result != -1;
	}

	/**
	 * This method is intended only for kt::SocketProtcol::UDP kt::Sockets.
	 * Usage on a *kt::SocketProtocol::TCP* socket will always return *false*.
	 * 
	 * @return true if this socket is bound, otherwise false. 
	 */
	bool Socket::isBound() const
	{
		return this->bound;
	}

	/**
	 * Reads and returns a single character from the reciever.
	 * 
	 * @return The character read.
	 */
	char Socket::get() const
	{
		return this->receiveAmount(1)[0];
	}

	/**
	 * @return the port number used by this socket.
	 */
	unsigned int Socket::getPort() const
	{
		return this->port;
	}

	/**
	 * @return the kt::SocketType for this kt::Socket.
	 */
	kt::SocketType Socket::getType() const
	{
		return this->type;
	}

	/**
	 * @return the kt::SocketProtocol configured for this kt::Socket.
	 */
	kt::SocketProtocol Socket::getProtocol() const
	{
		return this->protocol;
	}

	/**
	 * This method is intended only for kt::SocketProtcol::UDP kt::Sockets.
	 * Use on a kt::SocketProtcol::TCP will result in an empty string.
	 * 
	 * @return when using *kt::SocketProtocol::UDP* the address of the last device who sent the data that was most recently read. Always returns an empty string for kt::SocketProtocol::TCP kt::Sockets.
	 */
	std::string Socket::getLastRecievedAddress() const
	{
		if (this->protocol == kt::SocketProtocol::UDP)
		{
			char ip[20];
			strcpy(ip, inet_ntoa(this->clientAddress.sin_addr));
			return std::string(ip);
		}
		return "";
	}

	/**
	 * @return the hostname configured for this socket.
	 */
	std::string Socket::getAddress() const
	{
		return this->hostname;
	}

	struct sockaddr_in Socket::getSendAddress()
	{
		struct sockaddr_in newAddress;
		memset(&newAddress, 0, sizeof(newAddress));

		if (this->protocol == kt::SocketProtocol::UDP)
		{
			if (this->isBound())
			{
				memcpy(&newAddress, &this->clientAddress, sizeof(this->clientAddress));
			}
			else
			{
				memcpy(&newAddress, &this->serverAddress, sizeof(this->serverAddress));
			}
		}
		return newAddress;
	}

	/**
	 * Reads in a specific amount of character from the input stream and returns them as a std::string.
	 * This method will return early if there is no more data to send or the other party closes the connection.
	 * 
	 * @param amountToReceive - The amount of characters to read from the sender.
	 * 
	 * @return A std::string of the specified size with the respective character read in. 
	 */
	std::string Socket::receiveAmount(const unsigned int amountToReceive) const
	{
		if (amountToReceive == 0 || !this->ready())
		{
			return "";
		}
		const unsigned int bufferSize = amountToReceive + 1;

		char data[bufferSize];
		memset(&data, 0, bufferSize);
		unsigned int counter = 0;
		int flag;
		std::string result;
		result.reserve(bufferSize - 1);

		if (this->protocol == kt::SocketProtocol::TCP)
		{
			do
			{
				flag = recv(this->socketDescriptor, data, static_cast<int>(amountToReceive - counter), 0);
				
				if (flag < 1)
				{
					return result;
				}
				result += std::string(data);
				counter += flag;
			} while (counter < amountToReceive && this->ready());
		}
		else if (this->protocol == kt::SocketProtocol::UDP)
		{
			// UDP is odd, and will consume the entire datagram after a single read even if not all bytes are read
			socklen_t addressLength = sizeof(this->clientAddress);
			flag = recvfrom(this->socketDescriptor, data, static_cast<int>(amountToReceive), 0, (struct sockaddr*)&this->clientAddress, &addressLength);

			if (flag < 1)
			{
				return result;
			}
			result += std::string(data);
		}
		return result;
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
	std::string Socket::receiveToDelimiter(const char delimiter) const
	{
		if (delimiter == '\0')
		{
			throw SocketException("The null terminator '\\0' is an invalid delimiter.");
		}

		std::string data;
		int flag;

		if (!this->ready())
		{
			return data;
		}

		if (this->protocol == kt::SocketProtocol::TCP)
		{
			char character;
			do
			{
				character = this->get();
				if (character != delimiter)
				{
					data += character;
				}
			} while (character != delimiter && this->ready());

			return data;
		}
		else if (this->protocol == kt::SocketProtocol::UDP)
		{
			char temp[this->udpMaxBufferSize + 1];
			memset(&temp, 0, sizeof(temp));
			socklen_t addressLength = sizeof(this->clientAddress);
			
			flag = recvfrom(this->socketDescriptor, temp, static_cast<int>(this->udpMaxBufferSize), 0, (struct sockaddr*)&this->clientAddress, &addressLength);

			if (flag < 1)
			{
				return data;
			}

			data += std::string(temp);
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
	 * This method may take some time for the receiver to retrieve the whole message due to the inability to flush streams when using sockets.
	 */
	std::string Socket::receiveAll(const unsigned long timeout) const
	{
		std::string result;
		result.reserve(1024);
		bool hitEOF = false;

		if (!this->ready())
		{
			return "";
		}

		while (this->ready(timeout) && !hitEOF)
		{
			std::string res = receiveAmount(this->pollSocket(timeout));
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
	std::vector<std::pair<std::string, std::string> > Socket::scanDevices(unsigned int duration)
	{
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
			memset(&deviceName, 0, sizeof(deviceName));
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
    }

	std::string Socket::getLocalMACAddress()
	{
		int id;
		bdaddr_t btaddr;
		char localMACAddress[18];
	
		// Get id of local device
		if ((id = hci_get_route(nullptr)) < 0)
		{
			return "";
		}
	
		// Get local bluetooth address
		if (hci_devba(id, &btaddr) < 0)
		{
			return "";
		}
	
		// Convert address to string
		if (ba2str(&btaddr, localMACAddress) < 0)
		{
			return "";
		}
		
		return std::string(localMACAddress);
	}

	/**
	 * Get the underlying max buffer size used to limit the amount of bytes received per `receiveToDelimiter()` call for UDP Sockets.
	 *
	 * @return underlying receiving buffer limit, default is 10240
	 */
    unsigned int Socket::getUdpMaxBufferSize() const
    {
        return this->udpMaxBufferSize;
    }

    /**
     * Set the underlying max buffer size used to limit the amount of bytes received per `receiveToDelimiter()` call for UDP Sockets.
     *
     * @param newLimit the new limit to set
     */
    void Socket::setUdpMaxBufferSize(const unsigned int& newLimit)
    {
        this->udpMaxBufferSize = newLimit;
    }

    /**
     * Takes an object of type T, serailises it using the provided serialiser then sends it through the underlying socket connection.
     *
     * @tparam T the type of the object that will be serialised
     *
     * @param object object of type T to serialise and send
     * @param serialiser the serialiser defining how the object of type T can be serialised
     * @param flag used in send
     * @return true if the object was serialised and sent successfully otherwise false
     */
    template<typename T>
    bool Socket::sendObject(T object, SocketSerialiser<T>* serialiser, int flag)
    {
        std::vector<char> bytes = serialiser->serialise(object);
        if (bytes.empty())
        {
            return false;
        }
        return this->send(bytes.data(), bytes.size(), flag);
    }

    /**
     * Read the underlying socket until the `\0` character is read. (Or the max bytes for UDP).
     * The read bytes will be transformed using the deserialiser into the object of type T and returned.
     *
     * For UDP Sockets, tweak the internal udpMaxBuffer to extend the UDP read size.
     *
     * @tparam T object of type T to be received and deserialised
     * @param serialiser serialiser defining how to deserialise object of type T
     * @return the deserialised object
     */
    template<typename T>
    T Socket::receiveObject(SocketSerialiser<T>* serialiser)
    {
        std::string bytes = this->receiveToDelimiter('\0');
        std::vector<char> vector;
        vector.reserve(bytes.size());
        vector.assign(bytes.begin(), bytes.end());
        return serialiser->deserialise(bytes.c_str());
    }


} // End namespace kt
