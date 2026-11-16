#pragma once

#include <iostream>

#include "SocketException.hpp"

namespace kt
{
	class BindingException : public kt::SocketException
	{
		public:
			BindingException(const std::string& s) : kt::SocketException(s) {}
	};
}
