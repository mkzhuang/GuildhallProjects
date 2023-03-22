#pragma once
#include "Engine/Net/NetAddress.hpp"

#include <stdint.h>

typedef uintptr_t SocketHandle;

class Socket
{
public:
	Socket();
	~Socket();

	void Close();
	bool isClosed() const;

	void SetBlocking(bool isBlocking);
	bool CheckForFatalError();
	std::string const GetAddressString();
	unsigned int const GetIP();

protected:
	SocketHandle m_handle;
	NetAddress m_address;
};


