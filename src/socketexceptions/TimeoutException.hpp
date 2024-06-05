#pragma once

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
