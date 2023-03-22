#include "Engine/Net/Socket.hpp"

#include <winsock2.h>

Socket::Socket()
	:m_handle(INVALID_SOCKET)
{
}


Socket::~Socket()
{
	Close();
}


void Socket::Close()
{
	if (!isClosed())
	{
		::closesocket(m_handle);
		m_handle = INVALID_SOCKET;
	}
}


bool Socket::isClosed() const
{
	return m_handle == INVALID_SOCKET;
}


void Socket::SetBlocking(bool isBlocking)
{
	u_long non_blocking = isBlocking ? 0 : 1;
	::ioctlsocket(m_handle, FIONBIO, &non_blocking);
}


bool Socket::CheckForFatalError()
{
	int error = WSAGetLastError();
	switch (error)
	{
		case 0 : // no error
		case WSAEWOULDBLOCK:
			return false;
		
		default: // disconnect due to #error 
			Close();
			return true;
	}
}


std::string const Socket::GetAddressString()
{
	return m_address.ToString();
}


unsigned int const Socket::GetIP()
{
	return m_address.address;
}


