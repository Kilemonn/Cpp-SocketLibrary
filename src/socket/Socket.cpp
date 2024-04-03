
#include "Socket.h"
#include "../socketexceptions/SocketException.hpp"
#include "../socketexceptions/BindingException.hpp"
#include "../enums/SocketProtocol.cpp"
#include "../enums/SocketType.cpp"

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

#define _WIN32_WINNT 0x501

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
	Socket::Socket(const std::string& hostname, const unsigned int& port, const kt::SocketType type, const kt::SocketProtocol protocol)
	{
		this->hostname = hostname;
		this->port = port;
		this->type = type;
		this->protocol = protocol;
		this->serverAddress = { 0 };
		this->socketDescriptor = 0;
		memset(&this->clientAddress, 0, sizeof(this->clientAddress));

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
		memset(&this->clientAddress, 0, sizeof(this->clientAddress));
		memset(&this->serverAddress, 0, sizeof(this->serverAddress));
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

		this->clientAddress = socket.clientAddress;
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
	Socket& Socket::operator=(const Socket& socket)
	{
		this->socketDescriptor = socket.socketDescriptor;
		this->hostname = socket.hostname;
		this->protocol = socket.protocol;
		this->port = socket.port;
		this->type = socket.type;

		this->clientAddress = socket.clientAddress;
		this->serverAddress = socket.serverAddress;
#ifdef __linux__
		this->bluetoothAddress = socket.bluetoothAddress;

#endif

		return *this;
	}

	void Socket::constructBluetoothSocket()
	{
#ifdef _WIN32
		throw SocketException("Socket:constructBluetoothSocket() is not supported on Windows.");

		/*this->socketDescriptor = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
		if (this->socketDescriptor == 0)
		{
			throw SocketException("Error establishing Bluetooth socket: " + std::string(std::strerror(errno)));
		}

		this->bluetoothAddress.addressFamily = AF_BTH;
		this->bluetoothAddress.btAddr = std::stoull(this->hostname);
		this->bluetoothAddress.port = this->port;

		if (connect(this->socketDescriptor, (struct sockaddr*)&this->bluetoothAddress, sizeof(SOCKADDR_BTH)) == -1)
		{
			throw SocketException("Error connecting to Bluetooth server: " + std::string(std::strerror(errno)));
		}*/

#elif __linux__
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
#endif
	}

	void Socket::constructWifiSocket()
	{
		const int socketFamily = AF_INET;
		const int socketType = this->protocol == kt::SocketProtocol::TCP ? SOCK_STREAM : SOCK_DGRAM;
		const int socketProtocol = this->protocol == kt::SocketProtocol::TCP ? IPPROTO_TCP : IPPROTO_UDP;
		
#ifdef _WIN32
		WSADATA wsaData;
		if (int res = WSAStartup(MAKEWORD(2, 2), &wsaData); res != 0)
		{
			throw SocketException("WSAStartup Failed. " + std::to_string(res));
		}
		
		this->socketDescriptor = socket(socketFamily, socketType, socketProtocol);
		if (this->socketDescriptor == 0)
		{
			throw SocketException("Error establishing Wifi socket: " + this->getErrorCode());
		}

#elif __linux__
		this->socketDescriptor = socket(socketFamily, socketType, 0);
	    if (this->socketDescriptor == -1)
	    {
	    	throw SocketException("Error establishing Wifi socket: " + this->getErrorCode());
	    }

#endif
		struct hostent* server = gethostbyname(this->hostname.c_str());
		if (!this->hostname.empty() && server != nullptr)
		{
			memset(&this->serverAddress, 0, sizeof(this->serverAddress));
			this->serverAddress.sin_family = AF_INET;
			memcpy((char*)&this->serverAddress.sin_addr.s_addr, (char*)server->h_addr, server->h_length);
			this->serverAddress.sin_port = htons(this->port);
		}
		else
		{
			throw SocketException("Unable to resolve IP of destination address with hostname: [" + this->hostname + "].");
		}

		if (this->protocol == kt::SocketProtocol::TCP)
		{
			if (int res = connect(this->socketDescriptor, (struct sockaddr*)&this->serverAddress, sizeof(this->serverAddress)); res == -1)
			{
				this->close();
				throw SocketException("Error connecting to Wifi server: [" + std::to_string(res) + "] " + this->getErrorCode());
			}
		}
	}

	std::string Socket::getErrorCode() const
	{
#ifdef _WIN32
		return std::to_string(WSAGetLastError());

#elif __linux__
		return std::string(std::strerror(errno));

#endif
	}

	/**
	 * Closes the existing connection. If no connection is open, then it will do nothing.
	 * This method should be called before the object itself is distructed.
	 */
	void Socket::close()
	{
#ifdef _WIN32
		closesocket(this->socketDescriptor);

#elif __linux__
		::close(this->socketDescriptor);
#endif

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
	bool Socket::bind()
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

			if (this->port == 0)
			{
#ifdef _WIN32
			// TODO

#elif __linux__
				socklen_t socketSize = sizeof(this->serverAddress);
				if (getsockname(this->socketDescriptor, (struct sockaddr*)&this->serverAddress, &socketSize) != 0)
				{
					this->close();
					throw BindingException("Unable to retrieve randomly bound port number during socket creation. " + std::string(std::strerror(errno)));
				}

				this->port = ntohs(this->serverAddress.sin_port);
			
#endif
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
			if (this->protocol == kt::SocketProtocol::TCP)
			{
				return ::send(this->socketDescriptor, message.c_str(), message.size(), flag) != -1;
			}
			else if (this->protocol == kt::SocketProtocol::UDP)
			{
				struct sockaddr_in address = this->getSendAddress();
				return ::sendto(this->socketDescriptor, message.c_str(), message.size(), flag, (const struct sockaddr *)&address, sizeof(address)) != -1;
			}
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
	std::optional<char> Socket::get()
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
	std::optional<std::string> Socket::getLastRecievedAddress() const
	{
		if (this->protocol == kt::SocketProtocol::UDP)
		{
			char ip[20];
			memset(&ip, 0, 20);
#ifdef _WIN32
			// TODO

#elif __linux__
			strcpy(ip, inet_ntoa(this->clientAddress.sin_addr));
#endif
			std::string asString(ip);
			// Since we zero out the address, we need to check its not default initialised
			return asString != "0.0.0.0" ? std::optional<std::string>{asString} : std::nullopt;
		}
		return std::nullopt;
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
	std::string Socket::receiveAmount(const unsigned int amountToReceive)
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
			socklen_t addressLength = sizeof(this->clientAddress);
			int flag = recvfrom(this->socketDescriptor, &data[0], static_cast<int>(amountToReceive), 0, (struct sockaddr*)&this->clientAddress, &addressLength);
			if (flag < 1)
			{
				return result;
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
	 * 
	 * @return A std::string with all of the characters preceeding the delimiter.
	 * 
	 * @throw SocketException - if the delimiter is '\0'.
	 */
	std::string Socket::receiveToDelimiter(const char delimiter)
	{
		if (delimiter == '\0')
		{
			throw SocketException("The null terminator '\\0' is an invalid delimiter.");
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
			temp.resize(this->MAX_BUFFER_SIZE);
			socklen_t addressLength = sizeof(this->clientAddress);
			
			int flag = recvfrom(this->socketDescriptor, &temp[0], static_cast<int>(this->MAX_BUFFER_SIZE), 0, (struct sockaddr*)&this->clientAddress, &addressLength);
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
	std::string Socket::receiveAll(const unsigned long timeout)
	{
		if (this->protocol == SocketProtocol::UDP)
		{
			throw SocketException("Socket::receiveAll(unsigned long) is not supported for UDP socket configuration. Please use Socket::receiveAmount(unsigned int) instead.");
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
#ifdef _WIN32
		throw SocketException("Socket::scanDevices(int) is not supported on Windows.");

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
#endif
	}

	std::optional<std::string> Socket::getLocalMACAddress()
	{
#ifdef _WIN32
		throw SocketException("Socket::getLocalMACAddress() is not supported on Windows.");

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
