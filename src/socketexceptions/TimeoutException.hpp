#pragma once

#include <iostream>
#include <stdexcept>

#include "SocketException.hpp"

namespace kt
{
	class TimeoutException : public kt::SocketException
	{
		public:
			TimeoutException(const std::string &s) : kt::SocketException(s) {}
	};
}
