
#ifndef _TIMEOUT_ERROR_HPP__
#define _TIMEOUT_ERROR_HPP__

#include <iostream>
#include <stdexcept>

#include "SocketException.hpp"

namespace kt
{
	class TimeoutException : public SocketException
	{
		public:
			TimeoutException(const std::string &s) : SocketException(s) {}
	};
}

#endif // _TIMEOUT_ERROR_HPP__
