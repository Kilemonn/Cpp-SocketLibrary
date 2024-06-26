#pragma once

#include <string>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
#endif

#include <WinSock2.h>

#elif __linux__

typedef int SOCKET;

#endif

namespace kt
{
	std::string getErrorCode();

	SOCKET getInvalidSocketValue();

	bool isInvalidSocket(SOCKET descriptor);
} // End kt namespace
