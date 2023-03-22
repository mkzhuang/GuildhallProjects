#pragma once
#include "Engine/Net/TCPSocket.hpp"

class TCPConnection;

class TCPServer : public TCPSocket
{
public:
	bool Host(uint16_t service, uint32_t backlog = 16);
	TCPConnection* Accept();
	uint16_t GetPort() const;
};