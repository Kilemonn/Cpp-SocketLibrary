#include "SocketAddress.h"

#include <optional>
#include <string>
#include <vector>

namespace kt
{
	kt::InternetProtocolVersion getInternetProtocolVersion(const kt::SocketAddress& address)
	{
		return static_cast<kt::InternetProtocolVersion>(address.address.sa_family);
	}

	unsigned int getPortNumber(const kt::SocketAddress& address)
	{
		kt::InternetProtocolVersion version = getInternetProtocolVersion(address);
		if (version == kt::InternetProtocolVersion::IPV6)
		{
			return htons(address.ipv6.sin6_port);
		}
		// I believe the address is in the same position for ipv4 and ipv6 structs, so it doesn't really matter.
		// Doing the checks anway to make sure its fine
		return htons(address.ipv4.sin_port);
	}

	std::optional<std::string> getAddress(const kt::SocketAddress& address)
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

	std::pair<std::optional<kt::SocketAddress>, int> socketToAddress(const SOCKET& socket)
	{
		kt::SocketAddress address{};
		socklen_t socketSize = sizeof(address);
		int result = getsockname(socket, &address.address, &socketSize);
		return std::make_pair(std::make_optional(address), result);
	}

	std::pair<std::vector<kt::SocketAddress>, int> resolveToAddresses(const std::optional<std::string>& hostname, const unsigned int& port, addrinfo& hints)
	{
		std::vector<kt::SocketAddress> addresses;
		addrinfo* resolvedAddresses = nullptr;

		int result = getaddrinfo(hostname.has_value() ? hostname.value().c_str() : nullptr, std::to_string(port).c_str(), &hints, &resolvedAddresses);
		if (result != 0 || resolvedAddresses == nullptr)
		{
			if (resolvedAddresses != nullptr)
			{
				freeaddrinfo(resolvedAddresses);
			}
			return std::make_pair(addresses, result);
		}

		// We need to iterate over the resolved address and attempt to connect to each of them, if a connection attempt is succesful 
		// we will return, otherwise we will throw is we are unable to connect to any.
		for (addrinfo* addr = resolvedAddresses; addr != nullptr; addr = addr->ai_next)
		{
			kt::SocketAddress address = {};
			std::memcpy(&address, addr->ai_addr, addr->ai_addrlen);
			addresses.push_back(address);
		}
		freeaddrinfo(resolvedAddresses);

		return std::make_pair(addresses, result);
	}

#ifdef _WIN32
	int getAddressLength(const kt::SocketAddress& address)
#else
	socklen_t getAddressLength(const kt::SocketAddress& address)
#endif
	{
		const kt::InternetProtocolVersion protocolVersion = getInternetProtocolVersion(address);
		return protocolVersion == kt::InternetProtocolVersion::IPV6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN;
	}
}
