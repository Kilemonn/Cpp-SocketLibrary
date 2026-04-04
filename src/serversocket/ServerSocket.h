#pragma once

#include "../socket/Socket.h"

namespace kt
{
	template <typename T>
	class ServerSocket : public Socket
	{
		public:
			virtual T accept(const long& = 0) const = 0;
	};
}
