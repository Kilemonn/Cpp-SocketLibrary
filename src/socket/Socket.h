

#ifndef _SOCKET_H__
#define _SOCKET_H__

#include <iostream>
#include <vector>
#include <utility>

#include "../template/SocketSerialisable.h"
#include "../enums/SocketProtocol.cpp"
#include "../enums/SocketType.cpp"

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>

namespace kt
{
	class Socket
	{
		private:
			std::string hostname;
			unsigned int port = 0;
			kt::SocketProtocol protocol = kt::SocketProtocol::None;
			kt::SocketType type = SocketType::None;
			bool bound = false;
			struct sockaddr_in clientAddress; // For UDP, stores the client address of the last message received
			int socketDescriptor = 0;
			struct sockaddr_in serverAddress; // For Wifi
			struct sockaddr_rc bluetoothAddress; // For Bluetooth
			unsigned int udpMaxBufferSize = 10240;

			void constructBluetoothSocket();
			void constructWifiSocket();
			struct sockaddr_in getSendAddress();
			int pollSocket(const unsigned long = 1000) const;

		public:
			Socket() = default;
			Socket(const std::string&, const unsigned int&, const kt::SocketType, const kt::SocketProtocol = kt::SocketProtocol::None); // Create Wi-Fi/Bluetooth Socket
			Socket(const int&, const kt::SocketType, const kt::SocketProtocol, const std::string&, const unsigned int&);

			Socket(const Socket&); // Copy Constructor
			Socket& operator=(const Socket&);
			
			bool bindUdpSocket();
			void close();
			
			bool ready(const unsigned long = 1000) const;
			bool connected(const unsigned long = 1000) const;
			bool send(const std::string&, int = 0);
            bool send(const char*, unsigned int, int = 0);

            template<typename T>
			bool sendObject(T, SocketSerialisable<T>, int = 0);
            template<typename T>
            T receiveObject(SocketSerialisable<T>);

			unsigned int getPort() const;
			bool isBound() const;
			kt::SocketProtocol getProtocol() const;
			kt::SocketType getType() const;
			std::string getLastRecievedAddress() const;
			std::string getAddress() const;
			unsigned int getUdpMaxBufferSize() const;
			void setUdpMaxBufferSize(const unsigned int&);

			char get() const;
			std::string receiveAmount(const unsigned int) const;
			std::string receiveToDelimiter(const char) const;
			std::string receiveAll(const unsigned long = 1000) const;

			static std::vector<std::pair<std::string, std::string> > scanDevices(unsigned int = 5);
			static std::string getLocalMACAddress();
	};

} // End namespace kt 

#endif //_SOCKET_H__
