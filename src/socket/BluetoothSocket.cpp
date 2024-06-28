#include "BluetoothSocket.h"
#include "../socketexceptions/SocketException.hpp"

namespace kt
{
	void BluetoothSocket::constructBluetoothSocket()
	{
		throw kt::SocketException("Socket:constructBluetoothSocket() is not supported.");
#ifdef _WIN32
		
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
		/*this->socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

		if (isInvalidSocket(this->socketDescriptor))
		{
			throw SocketException("Error establishing Bluetooth socket: " + std::string(std::strerror(errno)));
		}

		this->bluetoothAddress.rc_family = AF_BLUETOOTH;
		this->bluetoothAddress.rc_channel = (uint8_t)port;
		str2ba(this->hostname.c_str(), &this->bluetoothAddress.rc_bdaddr);

		if (connect(this->socketDescriptor, (sockaddr*)&this->bluetoothAddress, sizeof(this->bluetoothAddress)) == -1)
		{
			throw SocketException("Error connecting to Bluetooth server: " + std::string(std::strerror(errno)));
		}*/
#endif
	}

	int BluetoothSocket::pollSocket(SOCKET socket, const long& timeout) const
	{
		return kt::pollSocket(socket, timeout);
	}

	void BluetoothSocket::close(SOCKET socket)
	{
		kt::close(socket);
	}

	BluetoothSocket::BluetoothSocket(const std::string& hostname, const unsigned int& port)
	{
		this->hostname = hostname;
		this->port = port;
	}

	void BluetoothSocket::close()
	{
		this->close(this->socketDescriptor);
	}

	bool BluetoothSocket::send(const std::string&, int)
	{
		return false;
	}

	unsigned int BluetoothSocket::getPort() const
	{
		return this->port;
	}

	std::string BluetoothSocket::getHostname() const
	{
		return this->hostname;
	}

	std::optional<char> BluetoothSocket::get(const int& flags) const
	{
		std::string received = this->receiveAmount(1, flags);
		if (received.empty())
		{
			return std::nullopt;
		}
		return received[0];
	}

	std::string BluetoothSocket::receiveAmount(const unsigned int& amount, const int& flags) const
	{
		return std::string();
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
	std::vector<std::pair<std::string, std::string> > BluetoothSocket::scanDevices(unsigned int duration)
	{
		throw kt::SocketException("Socket::scanDevices(int) is not supported.");

#ifdef _WIN32

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

		/*std::vector<std::pair<std::string, std::string> > devices;
		std::pair<std::string, std::string> tempPair;

		inquiry_info* ii = nullptr;
		int maxResponse = 255, numberOfResponses, ownId, tempSocket, flags;
		char deviceAddress[19];
		char deviceName[248];

		ownId = hci_get_route(nullptr);
		tempSocket = hci_open_dev(ownId);
		if (ownId < 0 || tempSocket < 0)
		{
			throw SocketException("Error opening Bluetooth socket for scanning...");
		}

		flags = IREQ_CACHE_FLUSH;
		ii = new inquiry_info[maxResponse * sizeof(inquiry_info)];

		numberOfResponses = hci_inquiry(ownId, duration, maxResponse, nullptr, &ii, flags);
		if (numberOfResponses < 0)
		{
			delete[]ii;
			throw SocketException("Error scanning for bluetooth devices");
		}

		for (int i = 0; i < numberOfResponses; i++)
		{
			ba2str(&(ii + i)->bdaddr, deviceAddress);
			memset(&deviceName, '\0', sizeof(deviceName));
			if (hci_read_remote_name(tempSocket, &(ii + i)->bdaddr, sizeof(deviceName), deviceName, 0) < 0)
			{
				strcpy(deviceName, "[unknown]");
			}

			tempPair = std::make_pair<std::string, std::string>(deviceAddress, deviceName);
			devices.push_back(tempPair);

		}

		delete[]ii;
		::close(tempSocket);

		return devices;*/
#endif
	}

	std::optional<std::string> kt::BluetoothSocket::getLocalMACAddress()
	{
		throw kt::SocketException("Socket::getLocalMACAddress() is not supported.");

#ifdef _WIN32

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
		// int id;
		// bdaddr_t btaddr;
		// char localMACAddress[18];

		// // Get id of local device
		// if ((id = hci_get_route(nullptr)) < 0)
		// {
		// 	return std::nullopt;
		// }

		// // Get local bluetooth address
		// if (hci_dev_req(id, &btaddr) < 0)
		// {
		// 	return std::nullopt;
		// }

		// // Convert address to string
		// if (ba2str(&btaddr, localMACAddress) < 0)
		// {
		// 	return std::nullopt;
		// }

		// return std::optional<std::string>{std::string(localMACAddress)};
#endif
	}
}

