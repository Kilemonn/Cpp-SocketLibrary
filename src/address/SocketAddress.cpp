#include "SocketAddress.h"
#include "../socketexceptions/SocketError.h"

#include <optional>
#include <string>
#include <vector>
#include <cstring> // Needed for std::memcpy()

namespace kt
{
	kt::InternetProtocolVersion kt::SocketAddress::getInternetProtocolVersion() const
	{
		kt::InternetProtocolVersion resolvedVersion = static_cast<kt::InternetProtocolVersion>(address.sa_family);
		
		if (resolvedVersion == kt::InternetProtocolVersion::IPV4 || resolvedVersion == kt::InternetProtocolVersion::IPV6)
		{
			return resolvedVersion;
		}
		return kt::InternetProtocolVersion::Any;
	}

	unsigned short kt::SocketAddress::getPortNumber() const
	{
		kt::InternetProtocolVersion version = getInternetProtocolVersion();
		if (version == kt::InternetProtocolVersion::IPV6)
		{
			return htons(ipv6.sin6_port);
		}
		// I believe the address is in the same position for ipv4 and ipv6 structs, so it doesn't really matter.
		// Doing the checks anway to make sure its fine
		return htons(ipv4.sin_port);
	}

	std::optional<std::string> kt::SocketAddress::getAddress() const
	{
		const kt::InternetProtocolVersion protocolVersion = getInternetProtocolVersion();
		const size_t addressLength = protocolVersion == kt::InternetProtocolVersion::IPV6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN;
		std::string asString;
		asString.resize(addressLength);

		if (protocolVersion == kt::InternetProtocolVersion::IPV6)
		{
			inet_ntop(static_cast<int>(protocolVersion), &ipv6.sin6_addr, asString.data(), addressLength);
		}
		else
		{
			inet_ntop(static_cast<int>(protocolVersion), &ipv4.sin_addr, asString.data(), addressLength);
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
	
#ifdef _WIN32
	int kt::SocketAddress::getAddressLength() const
#else
	socklen_t kt::SocketAddress::getAddressLength() const
#endif
	{
		const kt::InternetProtocolVersion protocolVersion = getInternetProtocolVersion();
		return protocolVersion == kt::InternetProtocolVersion::IPV4 ? sizeof(ipv4) : sizeof(ipv6);
	}

	std::expected<kt::SocketAddress, int> socketToAddress(const SOCKET& socket)
	{
		kt::SocketAddress address{};
		socklen_t socketSize = sizeof(address);
		int result = getsockname(socket, &address.address, &socketSize);
		if (result == -1)
		{
			return std::unexpected(kt::getErrorCodeValue());
		}
		return address;
	}

	std::expected<std::vector<kt::SocketAddress>, int> resolveToAddresses(const std::string& hostname, const unsigned short& port, addrinfo& hints)
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
			return std::unexpected(kt::getErrorCodeValue());
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

		return addresses;
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
}
