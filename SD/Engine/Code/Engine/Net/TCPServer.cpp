#include "Engine/Net/TCPServer.hpp"
#include "Engine/Net/TCPConnection.hpp"
#include <WinSock2.h>

bool TCPServer::Host(uint16_t service, uint32_t backlog)
{
	NetAddress hostAddress;
	hostAddress.address = INADDR_ANY;
	hostAddress.port = service;

	sockaddr_in addr_ipv4;
	addr_ipv4.sin_family = AF_INET;
	addr_ipv4.sin_addr.S_un.S_addr = ::htonl(hostAddress.address);
	addr_ipv4.sin_port = ::htons(hostAddress.port);

	SOCKET newSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (newSocket == INVALID_SOCKET)
	{
		return false;
	}

	int result = ::bind(newSocket, (sockaddr*) &addr_ipv4, (int) sizeof(addr_ipv4));
	if (result == SOCKET_ERROR)
	{
		::closesocket(newSocket);
		return false;
	}

	//SetBlocking(false);
	m_handle = newSocket;
	m_address = hostAddress;

	result = ::listen(m_handle, backlog);
	if (result == SOCKET_ERROR)
	{
		Close();
		return false;
	}

	return true;
}

TCPConnection* TCPServer::Accept()
{
	if (isClosed())
	{
		return nullptr;
	}

	sockaddr_storage addr;
	int addrlen = sizeof(addr);

	SocketHandle handle = ::accept(m_handle, (sockaddr*) &addr, &addrlen);
	if (handle == INVALID_SOCKET)
	{
		return nullptr;
	}

	if (addr.ss_family != AF_INET)
	{
		::closesocket(handle);
		return nullptr;
	}

	sockaddr_in* addr_ipv4 = (sockaddr_in*) &addr;
	NetAddress netAddress;
	netAddress.address = ::ntohl(addr_ipv4->sin_addr.S_un.S_addr);
	netAddress.port = ::ntohs(addr_ipv4->sin_port);

	TCPConnection* conn = new TCPConnection();
	conn->m_handle = handle;
	conn->m_address = netAddress;

	return conn;
}


uint16_t TCPServer::GetPort() const
{
	return m_address.port;
}


