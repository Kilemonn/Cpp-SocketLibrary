#include <gtest/gtest.h>
#include <optional>
#include <vector>

#include "../../src/address/SocketAddress.h"
#include "../../src/enums/InternetProtocolVersion.h"

namespace kt
{
	TEST(SocketAddressTest, SocketAddressGetInternetProtocolVersion_DefaultStruct)
	{
		kt::SocketAddress address{};
		ASSERT_EQ(kt::InternetProtocolVersion::Any, kt::getInternetProtocolVersion(address));
	}

	TEST(SocketAddressTest, SocketAddressGetInternetProtocolVersion_IPV4)
	{
		const std::string localhost = "localhost";
		const unsigned int port = 0;
		kt::InternetProtocolVersion version = kt::InternetProtocolVersion::IPV4;

		addrinfo hints{};
		hints.ai_family = static_cast<int>(version);

		std::pair<std::vector<kt::SocketAddress>, int> results = kt::resolveToAddresses(std::make_optional(localhost), port, hints);
		ASSERT_FALSE(results.first.empty());
		kt::SocketAddress firstAddress = results.first.at(0);
		ASSERT_EQ(version, kt::getInternetProtocolVersion(firstAddress));
	}

	TEST(SocketAddressTest, SocketAddressGetInternetProtocolVersion_IPV6)
	{
		const std::string localhost = "localhost";
		const unsigned int port = 0;
		kt::InternetProtocolVersion version = kt::InternetProtocolVersion::IPV6;

		addrinfo hints{};
		hints.ai_family = static_cast<int>(version);

		std::pair<std::vector<kt::SocketAddress>, int> results = kt::resolveToAddresses(std::make_optional(localhost), port, hints);
		ASSERT_FALSE(results.first.empty());
		kt::SocketAddress firstAddress = results.first.at(0);
		ASSERT_EQ(version, kt::getInternetProtocolVersion(firstAddress));
	}

	TEST(SocketAddressTest, SocketAddressGetInternetProtocolVersion_RandomSocketFamily)
	{
		kt::SocketAddress address{};
		address.address.sa_family = 1;

		ASSERT_NE(address.address.sa_family, static_cast<int>(kt::InternetProtocolVersion::Any));
		ASSERT_NE(address.address.sa_family, static_cast<int>(kt::InternetProtocolVersion::IPV4));
		ASSERT_NE(address.address.sa_family, static_cast<int>(kt::InternetProtocolVersion::IPV6));

		ASSERT_EQ(kt::InternetProtocolVersion::Any, kt::getInternetProtocolVersion(address));
	}

	TEST(SocketAddressTest, SocketAddressGetPortNumber_NoPortNumberSet)
	{
		kt::SocketAddress address{};
		ASSERT_EQ(0, kt::getPortNumber(address));
	}

	TEST(SocketAddressTest, SocketAddressGetPortNumber_IPV4PortAndFamily)
	{
		const unsigned short port = 57323;
		kt::SocketAddress address{};
		address.address.sa_family = static_cast<int>(kt::InternetProtocolVersion::IPV4);

		address.ipv4.sin_port = htons(port);
		ASSERT_EQ(port, kt::getPortNumber(address));
	}

	TEST(SocketAddressTest, SocketAddressGetPortNumber_IPV4PortAndIPV6Family)
	{
		const unsigned short port = 3248;
		kt::SocketAddress address{};
		address.address.sa_family = static_cast<int>(kt::InternetProtocolVersion::IPV6);

		address.ipv4.sin_port = htons(port);
		ASSERT_EQ(port, kt::getPortNumber(address));
	}

	TEST(SocketAddressTest, SocketAddressGetPortNumber_IPV6PortAndFamily)
	{
		const unsigned short port = 64123;
		kt::SocketAddress address{};
		address.address.sa_family = static_cast<int>(kt::InternetProtocolVersion::IPV6);

		address.ipv6.sin6_port = htons(port);
		ASSERT_EQ(port, kt::getPortNumber(address));
	}

	TEST(SocketAddressTest, SocketAddressGetPortNumber_IPV6PortAndIPV4Family)
	{
		const unsigned short port = 37652;
		kt::SocketAddress address{};
		address.address.sa_family = static_cast<int>(kt::InternetProtocolVersion::IPV4);

		address.ipv6.sin6_port = htons(port);
		ASSERT_EQ(port, kt::getPortNumber(address));
	}
}
