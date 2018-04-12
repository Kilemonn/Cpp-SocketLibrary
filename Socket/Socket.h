

#ifndef _SOCKET_H__
#define _SOCKET_H__

#include <iostream>
#include <vector>
#include <utility>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define _WIN32_WINNT 0x501

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#include "../includes/guiddef.h"
#include "../includes/ws2bth.h"

#elif __linux__

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>

#endif // _WIN32 / __linux__

namespace kt
{
	class Socket
	{
		private:
			std::string hostname;
			unsigned int port;
			bool isWifi;

			#ifdef _WIN32

			// Wifi Properties
			struct addrinfo *serverAddress;
	        struct addrinfo hints;
	    	SOCKET socketDescriptor;

	    	SOCKADDR_BTH bluetoothAddress;

			#elif __linux__

			int socketDescriptor;
			struct sockaddr_in serverAddress; // For Wifi
			struct sockaddr_rc bluetoothAddress; // For Bluetooth

			#endif // _WIN32 / __linux__

			void constructBluetoothSocket();
			void constructWifiSocket();

		public:
			const static bool WIFI = true;
			const static bool BLUETOOTH = false;

			Socket();
			Socket(const std::string&, const unsigned int&, const bool); // Create Wi-Fi/Bluetooth Socket
			
			#ifdef _WIN32
			
			Socket(const SOCKET&, const bool);

			#elif __linux__

			Socket(const int&, const bool); // Construct Socket from just descriptor

			#endif

			Socket(const Socket&); // Copy Constructor
			Socket& operator=(const Socket&);
			~Socket();
			
			void close();
			bool ready() const;
			bool send(const std::string, int flag = 0) const;
			char get() const;
			std::string receiveAmount(const unsigned int) const;
			std::string receiveToDelimiter(const char) const;
			std::string receiveAll() const;

			static std::vector<std::pair<std::string, std::string> > scanDevices(unsigned int duration = 5);
	};

} // End namespace kt 

#endif //_SOCKET_H__
