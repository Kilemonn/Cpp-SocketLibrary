#pragma once

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
#endif

#include <WinSock2.h>

#else

#include <sys/socket.h>
#include <sys/time.h>

// Typedef to match the windows typedef since they are different underlying types
typedef int SOCKET;

#endif

namespace kt
{
	int pollSocket(const SOCKET& socketDescriptor, const long& timeout, timeval* timeOutVal = nullptr);

	void close(SOCKET socket);

} // End namespace kt 
