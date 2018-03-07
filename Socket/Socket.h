

#ifndef _SOCKET_H__
#define _SOCKET_H__

#include <iostream>
#include <vector>
#include <utility>

#ifdef _WIN32

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#elif __linux__

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>

#endif // Platform

class Socket
{
	private:
		std::string hostname;
		unsigned int port;
		bool isWifi;

		#ifdef _WIN32

		// Wifi Properties
		struct addrinfo *result = nullptr;
        struct addrinfo *ptr = nullptr;
        struct addrinfo hints;
    	SOCKET ConnectSocket = INVALID_SOCKET;

		#elif __linux__

		int socketDescriptor;
		struct sockaddr_in serverAddress; // For Wifi
		struct sockaddr_rc bluetoothAddress; // For Bluetooth

		#endif // Platform

		void constructBluetoothSocket();
		void constructWifiSocket();

	public:
		const static bool WIFI = true;
		const static bool BLUETOOTH = false;

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
		bool send(const std::string, int severity = 0) const;
		char get() const;
		std::string receiveAmount(const unsigned int) const;
		std::string receiveToDelimiter(const char) const;

		static std::vector<std::pair<std::string, std::string> > scanDevices(unsigned int duration = 5);
};

#endif //_SOCKET_H__
