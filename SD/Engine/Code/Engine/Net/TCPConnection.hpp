#pragma once
#include "Engine/Net/TCPSocket.hpp"
#include "Engine/Net/NetAddress.hpp"

constexpr int BUFFER_SIZE = 4096;

enum class ConnectionState
{
	Disconnected,
	Connecting,
	Connected,
};

class TCPConnection : public TCPSocket
{

	friend class TCPServer;

public:
	bool Connect(NetAddress const& address);
	bool IsConnected();
	bool CheckForConnection();
	size_t Send(void const* data, size_t const dataSize);
	bool ReceiveFull(void* data);
	size_t Receive(void* data, size_t const maxDataSize);

	std::string GetHistory() const;
	void SetHistory(std::string const& history);

private:
	ConnectionState m_state = ConnectionState::Disconnected;
	size_t m_bytesRemain = 0;
	size_t m_bytesRead = 0;
	char m_buffer[BUFFER_SIZE];
	std::string m_history = "";
};


