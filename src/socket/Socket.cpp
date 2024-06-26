
#include "Socket.h"
#include "../socketexceptions/SocketException.hpp"
#include "../socketexceptions/BindingException.hpp"
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
		
		timeOutVal->tv_usec = timeout;

		FD_ZERO(&sReady);
		FD_SET(socketDescriptor, &sReady);

		// Need this->socketDescriptor + 1 here
		return select(socketDescriptor + 1, &sReady, nullptr, nullptr, timeOutVal);
	}

	void close(SOCKET socket)
	{
#ifdef _WIN32
		closesocket(socket);

#elif __linux__
		::close(socket);
#endif
	}

} // End namespace kt
