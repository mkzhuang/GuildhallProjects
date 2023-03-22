#include "Engine/Net/TCPConnection.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <winsock2.h>

#define UNUSED(x) (void)(x);

bool TCPConnection::Connect(NetAddress const& address)
{
	// initialize
	sockaddr_in addr_ipv4;

	unsigned int uintAddr = address.address;
	uint16_t port = address.port;

	addr_ipv4.sin_family = AF_INET;
	addr_ipv4.sin_addr.S_un.S_addr = ::htonl(uintAddr);
	addr_ipv4.sin_port = ::htons(port);

	// create socket
	SOCKET newSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (newSocket == INVALID_SOCKET)
	{
		return false;
	}

	int result = ::connect(newSocket, (sockaddr*) &addr_ipv4, (int) sizeof(addr_ipv4));
	if (result == SOCKET_ERROR)
	{
		int error = ::WSAGetLastError();
		if (error == WSAEWOULDBLOCK)
		{
			m_state = ConnectionState::Connecting;
			return true;
		}
		else
		{
			::closesocket(newSocket);
			return false;
		}
	}

	m_handle = newSocket;
	m_address = address;

	m_state = ConnectionState::Connected;

	return true;
}


bool TCPConnection::IsConnected()
{
	return m_state == ConnectionState::Connected;
}


bool TCPConnection::CheckForConnection()
{
	if (IsConnected())
	{
		return true;
	}

	if (isClosed())
	{
		return false;
	}

	WSAPOLLFD poll;
	poll.fd = m_handle;
	poll.events = POLLWRNORM;

	int result = ::WSAPoll(&poll, 1, 0);
	if (result == SOCKET_ERROR)
	{
		Close();
		return false;
	}

	if ((poll.revents & POLLHUP) != 0)
	{
		Close();
		return false;
	}

	if ((poll.revents & POLLWRNORM) != 0)
	{
		m_state = ConnectionState::Connected;
		return true;
	}

	return true;
}


size_t TCPConnection::Send(void const* data, size_t const dataSize)
{
	if (isClosed())
	{
		return 0;
	}

	int bytesSent = ::send(m_handle, (char const*) data, (int) dataSize, 0);
	if (bytesSent > 0)
	{
		GUARANTEE_OR_DIE(bytesSent == dataSize, "data size invalid for sending data");
		return bytesSent;
	}
	else if (bytesSent == 0)
	{
		Close();
		return 0;
	}
	else
	{
		Close();
		return 0;
	}
}


bool TCPConnection::ReceiveFull(void* data)
{
 	if (m_bytesRemain == 0)
	{
		if (m_bytesRead == 0)
		{
			size_t payloadSize;
			size_t recvd = Receive(&payloadSize, 2);
			if (recvd == 2)
			{
				m_bytesRemain = ::ntohs(static_cast<u_short>(payloadSize));
			}
			return false;
		}
		else
		{
			memcpy(data, m_buffer, m_bytesRead);
			m_bytesRead = 0;
			m_bytesRemain = 0;
			memset(m_buffer, 0, BUFFER_SIZE);
			return true;
		}
	}
	else
	{
		size_t recvd = Receive(m_buffer + m_bytesRead, m_bytesRemain);
		m_bytesRead += recvd;
		m_bytesRemain -= m_bytesRead;
		return false;
	}
}


size_t TCPConnection::Receive(void* data, size_t const maxDataSize)
{
	int bytesRead = ::recv(m_handle, (char*) data, (int) maxDataSize, 0);
	
	if (bytesRead > 0)
	{
		return (size_t)bytesRead;
	}
	else if (bytesRead == 0)
	{
		m_state = ConnectionState::Disconnected;
		Close();
		return 0;
	}
	else
	{
		CheckForFatalError();
		return 0;
	}
}

std::string TCPConnection::GetHistory() const
{
	return m_history;
}


void TCPConnection::SetHistory(std::string const& history)
{
	m_history = history;
}


