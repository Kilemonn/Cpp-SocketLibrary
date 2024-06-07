#pragma once

#include <iostream>
#include <stdexcept>

#include "SocketException.hpp"

namespace kt
{
	class BindingException : public SocketException
	{
		public:
			BindingException(const std::string& s) : SocketException(s) {}
	};
}
