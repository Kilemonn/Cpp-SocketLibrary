
#ifndef _TIMEOUT_EXCEPTION_HPP__
#define _TIMEOUT_EXCEPTION_HPP__

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

#endif // _TIMEOUT_EXCEPTION_HPP__
