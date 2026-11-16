#pragma once

#include "SocketException.hpp"

namespace kt
{
	class TimeoutException : public kt::SocketException
	{
		public:
			TimeoutException(const std::string &s) : kt::SocketException(s) {}
	};
}
