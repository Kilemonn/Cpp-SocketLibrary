#pragma once

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
