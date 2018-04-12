
#ifndef _SOCKET_ERROR_HPP__
#define _SOCKET_ERROR_HPP__

#include <iostream>
#include <stdexcept>

class SocketException : public std::runtime_error
{
	public:
		SocketException(const std::string& s) : std::runtime_error(s) {}
};

#endif // _SOCKET_ERROR_HPP__
