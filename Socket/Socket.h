

#ifndef _SOCKET_H__
#define _SOCKET_H__

#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>

class Socket
{
	private:
		std::string hostname;
		int socketDescriptor, port;
		struct sockaddr_in serverAddress; // For Wifi
		struct sockaddr_rc bluetoothAddress; // For Bluetooth
		bool isWifi;

		void constructBluetoothSocket();
		void constructWifiSocket();

	public:
		Socket(const std::string, const int, const bool); // Create Wi-Fi/Bluetooth Socket
		Socket(const int, const bool); // Construct Socket from just descriptor
		Socket(const Socket&); // Copy Constructor
		
		void closeSocket();
		bool sendMessage(const std::string, int severity = 0) const;
		std::string receiveAmount(int) const;
		std::string receiveToDelimiter(const char) const;
};

#endif //_SOCKET_H__
