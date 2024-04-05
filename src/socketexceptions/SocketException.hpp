
#ifndef _SOCKET_EXCEPTION_HPP__
#define _SOCKET_EXCEPTION_HPP__

#include <iostream>
#include <stdexcept>

namespace kt
{
	class SocketException : public std::runtime_error
	{
		public:
			SocketException(const std::string& s) : std::runtime_error(s) {}
	};
}

#endif // _SOCKET_EXCEPTION_HPP__
