#include "BluetoothSocket.h"
#include "../socketexceptions/SocketException.hpp"
#include <sstream>
#include <iomanip>

#ifdef _WIN32

#include "Windows.h"
#include <iphlpapi.h>
#include <ws2bth.h>
#include "bluetoothapis.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Bthprops.lib")

#elif __linux__

#endif

namespace kt
{
	void BluetoothSocket::constructBluetoothSocket()
	{
#ifdef _WIN32
		WSADATA wsaData{};
		if (int res = WSAStartup(MAKEWORD(2, 2), &wsaData); res != 0)
		{
			throw kt::SocketException("WSAStartup Failed. " + std::to_string(res));
		}
		
		this->socketDescriptor = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
		if (isInvalidSocket(this->socketDescriptor))
		{
			throw SocketException("Error establishing Bluetooth socket: " + std::string(std::strerror(errno)));
		}

		SOCKADDR_BTH bluetoothAddress{};
		bluetoothAddress.addressFamily = AF_BTH;
		bluetoothAddress.btAddr = std::stoull(this->hostname);
		bluetoothAddress.port = this->port;

		if (connect(this->socketDescriptor, (sockaddr*)&bluetoothAddress, sizeof(bluetoothAddress)) == -1)
		{
			throw SocketException("Error connecting to Bluetooth server: " + std::string(std::strerror(errno)));
		}

#elif __linux__
		throw kt::SocketException("Socket:constructBluetoothSocket() is not supported.");
		this->socketDescriptor = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

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
		}
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

	BluetoothSocket::BluetoothSocket(const std::string& hostname, const unsigned short& port)
	{
		this->hostname = hostname;
		this->port = port;
	}

	BluetoothSocket::BluetoothSocket(const SOCKET& socket, const std::string& hostname, const unsigned short& port)
	{
		this->socketDescriptor = socket;
		this->hostname = hostname;
		this->port = port;
	}

	void BluetoothSocket::close()
	{
		this->close(this->socketDescriptor);
	}

	bool BluetoothSocket::send(const std::string& s, int i)
	{
		return false;
	}

	unsigned short BluetoothSocket::getPort() const
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
	std::vector<std::pair<std::string, std::string>> BluetoothSocket::scanDevices(unsigned int duration)
	{
		std::vector<std::pair<std::string, std::string>> devices;
#ifdef _WIN32

		WSADATA wsaData{};
		int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (res != 0)
		{
			throw SocketException("WSAStartup Failed: " + std::to_string(res));
		}

		BLUETOOTH_DEVICE_SEARCH_PARAMS params{};
		params.dwSize = sizeof(params);
		params.fReturnAuthenticated = false;
		params.fReturnRemembered = false;
		params.fReturnConnected = true;
		params.fReturnUnknown = true;
		params.fIssueInquiry = true;
		params.cTimeoutMultiplier = duration;
		BLUETOOTH_DEVICE_INFO device{};
		HBLUETOOTH_DEVICE_FIND deviceFind = BluetoothFindFirstDevice(&params, &device);
		if (deviceFind == nullptr)
		{
			return devices;
		}

		do
		{
			std::string address = std::to_string(device.Address.ullLong);
			std::wstring name = std::wstring(device.szName);
			std::wcout << "Name: " << name << " address: " << std::flush;
			std::cout << address << std::endl;
			
		} while (BluetoothFindNextDevice(deviceFind, &device));

		BluetoothFindDeviceClose(deviceFind);

		return devices;

#elif __linux__

		throw kt::SocketException("Socket::scanDevices(int) is not supported.");

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
#ifdef _WIN32
		WSADATA wsaData{};
		if (int res = WSAStartup(MAKEWORD(2, 2), &wsaData); res != 0)
		{
			throw kt::SocketException("WSAStartup Failed: " + std::to_string(res));
		}

		BLUETOOTH_FIND_RADIO_PARAMS btfrp{};
		btfrp.dwSize = sizeof(btfrp);
		HANDLE hRadio{};
		HBLUETOOTH_RADIO_FIND hFind = BluetoothFindFirstRadio(&btfrp, &hRadio);
		if (hFind == nullptr)
		{
			//DWORD err = GetLastError();
			//switch (err)
			//{
			//case ERROR_NO_MORE_ITEMS:
			//	// No bluetooth radio found
			//	break;
			//default:
			//	// Error finding radios
			//	break;
			//}
			return std::nullopt;
		}

		do
		{
			BLUETOOTH_RADIO_INFO radioInfo{};
			radioInfo.dwSize = sizeof(radioInfo);
			DWORD res = BluetoothGetRadioInfo(hRadio, &radioInfo);
			if (res == ERROR_SUCCESS)
			{
				// The mac address is in radioInfo.address
				BluetoothFindRadioClose(hFind);
				return std::to_string(radioInfo.address.ullLong);
			}
		} while (BluetoothFindNextRadio(hFind, &hRadio));

		BluetoothFindRadioClose(hFind);

		return std::nullopt;

#elif __linux__
		throw kt::SocketException("Socket::getLocalMACAddress() is not supported.");
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

