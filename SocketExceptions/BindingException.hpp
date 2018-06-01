

#ifndef _BINDING_ERROR_HPP__
#define _BINDING_ERROR_HPP__

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

#endif // _BINDING_ERROR_HPP__
