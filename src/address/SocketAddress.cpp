#include "SocketAddress.h"

namespace kt
{
	kt::InternetProtocolVersion getInternetProtocolVersion(kt::SocketAddress& address)
	{
		return static_cast<kt::InternetProtocolVersion>(address.address.sa_family);
	}
}
