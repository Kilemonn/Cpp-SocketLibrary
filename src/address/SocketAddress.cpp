#include "SocketAddress.h"

#include <optional>
#include <string>
#include <vector>

namespace kt
{
	kt::InternetProtocolVersion getInternetProtocolVersion(const kt::SocketAddress& address)
	{
		kt::InternetProtocolVersion resolvedVersion = static_cast<kt::InternetProtocolVersion>(address.address.sa_family);
		
		if (resolvedVersion == kt::InternetProtocolVersion::IPV4 || resolvedVersion == kt::InternetProtocolVersion::IPV6)
		{
			return resolvedVersion;
		}
		return kt::InternetProtocolVersion::Any;
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
		return !asString.empty() && asString != getEmptyAddress(protocolVersion) ? std::optional<std::string>{asString} : std::nullopt;
	}

	std::pair<std::optional<kt::SocketAddress>, int> socketToAddress(const SOCKET& socket)
	{
		kt::SocketAddress address{};
		socklen_t socketSize = sizeof(address);
		int result = getsockname(socket, &address.address, &socketSize);
		return std::make_pair(result == -1 ? std::nullopt : std::make_optional(address), result);
	}

	std::pair<std::vector<kt::SocketAddress>, int> resolveToAddresses(const std::string& hostname, const unsigned short& port, addrinfo& hints)
	{
		std::vector<kt::SocketAddress> addresses;
		addrinfo* resolvedAddresses = nullptr;

		int result = getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &resolvedAddresses);
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

	addrinfo createUdpHints(const kt::InternetProtocolVersion protocolVersion, const int aiFlags)
	{
		addrinfo hints{};
		hints.ai_family = static_cast<int>(protocolVersion);
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
		hints.ai_flags = aiFlags;

		return hints;
	}

    addrinfo createTcpHints(const kt::InternetProtocolVersion protocolVersion, const int aiFlags)
	{
		addrinfo hints{};
		hints.ai_family = static_cast<int>(protocolVersion);
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = aiFlags;

		return hints;
	}

	std::optional<std::string> getEmptyAddress(const kt::InternetProtocolVersion protocolVersion)
	{
		if (protocolVersion == kt::InternetProtocolVersion::IPV4)
		{
			return std::make_optional("0.0.0.0");
		}
		else if (protocolVersion == kt::InternetProtocolVersion::IPV6)
		{
			return std::make_optional("::");
		}

		return std::nullopt;
	}

	std::string getLocalAddress(const kt::InternetProtocolVersion protocolVersion)
	{
		if (protocolVersion == kt::InternetProtocolVersion::IPV4)
		{
			return "127.0.0.1";
		}
		else if (protocolVersion == kt::InternetProtocolVersion::IPV6)
		{
			return "::1";
		}

		return "localhost";
	}

#ifdef _WIN32
	int getAddressLength(const kt::SocketAddress& address)
#else
	socklen_t getAddressLength(const kt::SocketAddress& address)
#endif
	{
		const kt::InternetProtocolVersion protocolVersion = getInternetProtocolVersion(address);
		return protocolVersion == kt::InternetProtocolVersion::IPV4 ? sizeof(address.ipv4) : sizeof(address.ipv6);
	}
}
