#include <gtest/gtest.h>
#include <optional>
#include <vector>

#include "../../src/socketexceptions/SocketError.h"
#include "../../src/address/SocketAddress.h"
#include "../../src/enums/InternetProtocolVersion.h"

namespace kt
{
	/**
	* Ensure a default initialised SocketAddress has "Any" set as the IP version.
	*/
	TEST(SocketAddressTest, SocketAddressGetInternetProtocolVersion_DefaultStruct)
	{
		kt::SocketAddress address{};
		ASSERT_EQ(kt::InternetProtocolVersion::Any, kt::getInternetProtocolVersion(address));
	}

	/**
	* Ensure that a SocketAddress with socket family set to IPV4 is resolved to the IPV4 enum.
	* Ensure the address returned is "127.0.0.1" for this test.
	*/
	TEST(SocketAddressTest, SocketAddressGetInternetProtocolVersionAndGetAddress_IPV4)
	{
		const std::string localhost = "localhost";
		const unsigned int port = 0;
		kt::InternetProtocolVersion version = kt::InternetProtocolVersion::IPV4;

		addrinfo hints = kt::createTcpHints(version);
		std::pair<std::vector<kt::SocketAddress>, int> results = kt::resolveToAddresses(std::make_optional(localhost), port, hints);
		
		ASSERT_FALSE(results.first.empty());
		kt::SocketAddress firstAddress = results.first.at(0);
		ASSERT_EQ(version, kt::getInternetProtocolVersion(firstAddress));

		ASSERT_EQ("127.0.0.1", kt::getAddress(firstAddress).value());
	}

	/**
	* Ensure that a SocketAddress with socket family set to IPV6 is resolved to the IPV6 enum.
	* Ensure the address returned is "::1" for this test.
	*/
	TEST(SocketAddressTest, SocketAddressGetInternetProtocolVersionAndGetAddress_IPV6)
	{
		const std::string localhost = "localhost";
		const unsigned int port = 0;
		kt::InternetProtocolVersion version = kt::InternetProtocolVersion::IPV6;

		addrinfo hints = kt::createUdpHints(version);
		std::pair<std::vector<kt::SocketAddress>, int> results = kt::resolveToAddresses(std::make_optional(localhost), port, hints);
		
		ASSERT_FALSE(results.first.empty());
		kt::SocketAddress firstAddress = results.first.at(0);
		ASSERT_EQ(version, kt::getInternetProtocolVersion(firstAddress));

		ASSERT_EQ("::1", kt::getAddress(firstAddress).value());
	}

	/**
	* Ensure that a SocketAddress with an undefined socket family is resolved to Any.
	*/
	TEST(SocketAddressTest, SocketAddressGetInternetProtocolVersion_RandomSocketFamily)
	{
		kt::SocketAddress address{};
		address.address.sa_family = 1;

		ASSERT_NE(address.address.sa_family, static_cast<int>(kt::InternetProtocolVersion::Any));
		ASSERT_NE(address.address.sa_family, static_cast<int>(kt::InternetProtocolVersion::IPV4));
		ASSERT_NE(address.address.sa_family, static_cast<int>(kt::InternetProtocolVersion::IPV6));

		ASSERT_EQ(kt::InternetProtocolVersion::Any, kt::getInternetProtocolVersion(address));
	}

	/**
	* Ensure a default SocketAddress resolves to 0 as the port number.
	*/
	TEST(SocketAddressTest, SocketAddressGetPortNumber_NoPortNumberSet)
	{
		kt::SocketAddress address{};
		ASSERT_EQ(0, kt::getPortNumber(address));
	}

	/**
	* Ensure that the port number can be retrieved if its set in the IPV4 struct and the family it set to IPV4.
	*/
	TEST(SocketAddressTest, SocketAddressGetPortNumber_IPV4PortAndFamily)
	{
		const unsigned short port = 57323;
		kt::SocketAddress address{};
		address.address.sa_family = static_cast<int>(kt::InternetProtocolVersion::IPV4);

		address.ipv4.sin_port = htons(port);
		ASSERT_EQ(port, kt::getPortNumber(address));
	}

	/**
	* Ensure that the port number can be retrieved if its set in the IPV4 struct and the family it set to IPV6.
	* 
	* This is confirming that the port is in the same position in the struct no matter which representation we use.
	*/
	TEST(SocketAddressTest, SocketAddressGetPortNumber_IPV4PortAndIPV6Family)
	{
		const unsigned short port = 3248;
		kt::SocketAddress address{};
		address.address.sa_family = static_cast<int>(kt::InternetProtocolVersion::IPV6);

		address.ipv4.sin_port = htons(port);
		ASSERT_EQ(port, kt::getPortNumber(address));
	}

	/**
	* Ensure that the port number can be retrieved if its set in the IPV6 struct and the family it set to IPV6.
	*/
	TEST(SocketAddressTest, SocketAddressGetPortNumber_IPV6PortAndFamily)
	{
		const unsigned short port = 64123;
		kt::SocketAddress address{};
		address.address.sa_family = static_cast<int>(kt::InternetProtocolVersion::IPV6);

		address.ipv6.sin6_port = htons(port);
		ASSERT_EQ(port, kt::getPortNumber(address));
	}

	/**
	* Ensure that the port number can be retrieved if its set in the IPV6 struct and the family it set to IPV4.
	*
	* This is confirming that the port is in the same position in the struct no matter which representation we use.
	*/
	TEST(SocketAddressTest, SocketAddressGetPortNumber_IPV6PortAndIPV4Family)
	{
		const unsigned short port = 37652;
		kt::SocketAddress address{};
		address.address.sa_family = static_cast<int>(kt::InternetProtocolVersion::IPV4);

		address.ipv6.sin6_port = htons(port);
		ASSERT_EQ(port, kt::getPortNumber(address));
	}

	/**
	* Ensure a default address gives the bigger IPV6 address size.
	*/
	TEST(SocketAddressTest, SocketAddressGetAddressLength_DefaultAddress)
	{
		kt::SocketAddress address{};
		ASSERT_EQ(sizeof(address.ipv6), kt::getAddressLength(address));
	}

	/**
	* Ensure an IPV6 family address returns the IPV6 address size.
	*/
	TEST(SocketAddressTest, SocketAddressGetAddressLength_IPV6Family)
	{
		kt::SocketAddress address{};
		address.address.sa_family = static_cast<int>(kt::InternetProtocolVersion::IPV6);
		ASSERT_EQ(sizeof(address.ipv6), kt::getAddressLength(address));
	}

	/**
	* Ensure an IPV4 family address returns the IPV4 address size.
	*/
	TEST(SocketAddressTest, SocketAddressGetAddressLength_IPV4Family)
	{
		kt::SocketAddress address{};
		address.address.sa_family = static_cast<int>(kt::InternetProtocolVersion::IPV4);
		ASSERT_EQ(sizeof(address.ipv4), kt::getAddressLength(address));
	}

	/**
	* Ensure a nullopt is returned with a default address.
	*/
	TEST(SocketAddressTest, SocketAddressGetAddress_DefaultAddress)
	{
		kt::SocketAddress address{};
		std::optional<std::string> result = kt::getAddress(address);
		ASSERT_EQ(std::nullopt, result);
	}

	/**
	* Ensure we get nullopt when we provide an invalid socket.
	*/
	TEST(SocketAddressTest, SocketAddressSocketToAddress_InvalidSocket)
	{
		SOCKET socket = kt::getInvalidSocketValue();
		std::pair<std::optional<kt::SocketAddress>, int> result = kt::socketToAddress(socket);
		ASSERT_EQ(std::nullopt, result.first);
		ASSERT_EQ(-1, result.second);
	}

	TEST(SocketAddressTest, SocketAddressResolveToAddresses_InvalidAddress)
	{
		std::string hostname = "cpp-socket-library.test.hostname";
		unsigned short port = 0;

		addrinfo hints = kt::createTcpHints(kt::InternetProtocolVersion::Any);
		std::pair<std::vector<kt::SocketAddress>, int> results = kt::resolveToAddresses(std::make_optional(hostname), port, hints);
		
		ASSERT_NE(0, results.second);
		ASSERT_TRUE(results.first.empty());
	}

	TEST(SocketAddressTest, SocketAddressResolveToAddresses_ResolveLocalhost)
	{
		std::string localhost = "localhost";
		unsigned short port = 0;

		addrinfo hints = kt::createUdpHints(kt::InternetProtocolVersion::Any);
		std::pair<std::vector<kt::SocketAddress>, int> results = kt::resolveToAddresses(std::make_optional(localhost), port, hints);
		
		ASSERT_EQ(0, results.second);
		ASSERT_FALSE(results.first.empty());
	}

	TEST(SocketAddressTest, TestAssignmentOperator)
	{
		std::string localhost = "localhost";
		addrinfo hints = kt::createTcpHints(kt::InternetProtocolVersion::Any);
		std::pair<std::vector<kt::SocketAddress>, int> results = kt::resolveToAddresses(std::make_optional(localhost), 0, hints);
		
		ASSERT_EQ(0, results.second);
		ASSERT_FALSE(results.first.empty());

		kt::SocketAddress address = results.first[0];
		kt::SocketAddress copiedAddress = address;
		ASSERT_NE(&address, &copiedAddress);
		ASSERT_EQ(0, std::memcmp(&address, &copiedAddress, sizeof(address)));
	}
}
