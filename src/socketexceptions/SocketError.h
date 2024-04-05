#ifndef _SOCKET_ERROR_HPP__
#define _SOCKET_ERROR_HPP__

#include <string>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define _WIN32_WINNT 0x501

#include <WinSock2.h>

#elif __linux__

typedef int SOCKET;

#endif

namespace kt
{
	std::string getErrorCode();

	bool isInvalidSocket(SOCKET descriptor);
} // End kt namespace

#endif // _SOCKET_ERROR_HPP__
