#ifndef _SOCKET_H__
#define _SOCKET_H__

#include <iostream>
#include <vector>
#include <utility>
#include <optional>

#include "../enums/SocketProtocol.cpp"
#include "../enums/SocketType.cpp"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define _WIN32_WINNT 0x501

#include <WinSock2.h>
#include <ws2bth.h>

#elif __linux__

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>

// Typedef to match the windows typedef since they are different underlying types
typedef int SOCKET;

#endif

namespace kt
{
	class Socket
	{
		private:
			std::string hostname;
			unsigned int port;
			kt::SocketProtocol protocol = kt::SocketProtocol::None;
			kt::SocketType type = SocketType::None;
			bool bound = false;
			sockaddr serverAddress; // For Wifi
			sockaddr_in clientAddress; // For UDP, stores the client address of the last message received
			SOCKET socketDescriptor = 0;

#ifdef _WIN32
			//SOCKADDR_BTH bluetoothAddress;

#elif __linux__
			sockaddr_rc bluetoothAddress; // For Bluetooth

#endif

			const unsigned int MAX_BUFFER_SIZE = 10240;

			void constructBluetoothSocket();
			void constructWifiSocket();
			sockaddr_in getSendAddress();
			int pollSocket(const unsigned long = 1000) const;

		public:
			Socket() = default;
			Socket(const std::string&, const unsigned int&, const kt::SocketType, const kt::SocketProtocol = kt::SocketProtocol::None); // Create Wi-Fi/Bluetooth Socket
			Socket(const SOCKET&, const kt::SocketType, const kt::SocketProtocol, const std::string&, const unsigned int&);

			Socket(const Socket&); // Copy Constructor
			Socket& operator=(const Socket&);
			
			bool bind();
			void close();
			
			bool ready(const unsigned long = 1000) const;
			bool connected(const unsigned long = 1000) const;
			bool send(const std::string&, int = 0);

			unsigned int getPort() const;
			bool isBound() const;
			kt::SocketProtocol getProtocol() const;
			kt::SocketType getType() const;
			std::optional<std::string> getLastRecievedAddress() const;
			std::string getAddress() const;

			std::optional<char> get();
			std::string receiveAmount(const unsigned int);
			std::string receiveToDelimiter(const char);
			std::string receiveAll(const unsigned long = 1000);

			static std::vector<std::pair<std::string, std::string> > scanDevices(unsigned int = 5);
			static std::optional<std::string> getLocalMACAddress();
	};

} // End namespace kt 

#endif //_SOCKET_H__
