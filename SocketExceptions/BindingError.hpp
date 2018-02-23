

#ifndef _BINDING_ERROR_HPP__
#define _BINDING_ERROR_HPP__

#include <iostream>
#include <stdexcept>

#include "SocketError.hpp"

class BindingError : public SocketError
{
	public:
		BindingError(const std::string& s) : SocketError(s) {}
};

#endif // _BINDING_ERROR_HPP__


