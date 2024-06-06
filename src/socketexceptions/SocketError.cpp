#include <string>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define _WIN32_WINNT 0x0600

#include <WinSock2.h>

#elif __linux__

#include <cstring> // std::strerror

typedef int SOCKET;

#endif

namespace kt
{
	std::string getErrorCode()
	{
#ifdef _WIN32
		return std::to_string(WSAGetLastError());

#elif __linux__
		return std::string(std::strerror(errno));

#endif
	}

	SOCKET getInvalidSocketValue()
	{
#ifdef _WIN32
		return INVALID_SOCKET;

#elif __linux__
		return -1;

#endif
	}

	bool isInvalidSocket(SOCKET descriptor)
	{
		return descriptor == getInvalidSocketValue();
	}

} // End kt namespace

