#pragma once

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#define _WIN32_WINNT 0x0600

#include <WinSock2.h>

#elif __linux__

#include <sys/socket.h>

// Typedef to match the windows typedef since they are different underlying types
typedef int SOCKET;

#endif

namespace kt
{
	int pollSocket(const SOCKET& socketDescriptor, const long& timeout, timeval* timeOutVal = nullptr);

	void close(SOCKET socket);

} // End namespace kt 
