#include "Engine/Net/NetAddress.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <WinSock2.h>
#include <ws2tcpip.h>

std::string NetAddress::ToString()
{
	return Stringf("%u.%u.%u.%u:%u",
		(address & 0xff000000) >> 24,
		(address & 0x00ff0000) >> 16,
		(address & 0x0000ff00) >> 8,
		(address & 0x000000ff) >> 0,
		port);
}


NetAddress NetAddress::FromString(std::string const& string)
{
	Strings parts = SplitStringOnDelimiter(string, ':');
	if (parts.size() != 2)
	{
		return NetAddress();
	}

	IN_ADDR addr;
	int result = ::inet_pton(AF_INET, parts[0].c_str(), &addr);
	if (result == SOCKET_ERROR)
	{
		return NetAddress();
	}

	uint16_t port = (uint16_t) ::atoi(parts[1].c_str());

	NetAddress address;
	address.address = ::ntohl(addr.S_un.S_addr);
	address.port = port;

	return address;
}


NetAddress NetAddress::GetLoopBack(uint16_t port)
{
	NetAddress addr;
	addr.address = 0x7f000001;
	addr.port = port;

	return addr;
}

std::vector<NetAddress> NetAddress::GetAllInternals(uint16_t port)
{
	std::vector<NetAddress> results;

	char hostname[256];
	int result = ::gethostname(hostname, sizeof(hostname));
	if (result != 0)
	{
		return results;
	}

	addrinfo hints = {};
	//memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	addrinfo* addresses = nullptr;
	int status = ::getaddrinfo(hostname, nullptr, &hints, &addresses);
	if (status != 0)
	{
		return results;
	}

	addrinfo* iter = addresses;
	while (iter)
	{
		NetAddress addr;
		sockaddr_in* ipv4 = (sockaddr_in*) iter->ai_addr;
		addr.address = ::ntohl(ipv4->sin_addr.S_un.S_addr);
		addr.port = port;
		results.push_back(addr);

		iter = iter->ai_next;
	}

	::freeaddrinfo(addresses);

	return results;
}

