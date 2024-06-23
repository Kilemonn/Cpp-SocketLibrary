#include "SocketAddress.h"

#include <optional>
#include <string>

namespace kt
{
	kt::InternetProtocolVersion getInternetProtocolVersion(const kt::SocketAddress& address)
	{
		return static_cast<kt::InternetProtocolVersion>(address.address.sa_family);
	}

	long getPortNumber(const kt::SocketAddress& address)
	{
		kt::InternetProtocolVersion version = getInternetProtocolVersion(address);
		if (version == kt::InternetProtocolVersion::IPV6)
		{
			return htonl(address.ipv6.sin6_port);
		}
		// I believe the address is in the same position for ipv4 and ipv6 structs, so it doesn't really matter.
		// Doing the checks anway to make sure its fine
		return htonl(address.ipv4.sin_port);
	}

	std::optional<std::string> resolveToAddress(const kt::SocketAddress& address)
	{
		const kt::InternetProtocolVersion protocolVersion = getInternetProtocolVersion(address);
		const size_t addressLength = protocolVersion == kt::InternetProtocolVersion::IPV6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN;
		std::string asString;
		asString.resize(addressLength);

		if (protocolVersion == kt::InternetProtocolVersion::IPV6)
		{
			inet_ntop(static_cast<int>(protocolVersion), &address.ipv6.sin6_addr, &asString[0], addressLength);
		}
		else
		{
			inet_ntop(static_cast<int>(protocolVersion), &address.ipv4.sin_addr, &asString[0], addressLength);
		}

		// Removing trailing \0 bytes
		const size_t delimiterIndex = asString.find_first_of('\0');
		if (delimiterIndex != std::string::npos)
		{
			asString = asString.substr(0, delimiterIndex);
		}
		// Since we zero out the address, we need to check its not default initialised
		return !asString.empty() && asString != "0.0.0.0" && asString != "::" ? std::optional<std::string>{asString} : std::nullopt;
	}
}
