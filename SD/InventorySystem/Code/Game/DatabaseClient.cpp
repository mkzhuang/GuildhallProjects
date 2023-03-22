#include "Game/DatabaseClient.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include <iostream>
#include <Winsock2.h>

DatabaseClient::DatabaseClient(DatabaseClientConfig const& config)
	: m_config(config)
	, m_console(config.console)
{
}


void DatabaseClient::Startup()
{
	if (m_config.serverAddr == "loopback")
	{
		NetAddress addr = NetAddress::GetLoopBack(3121);
		AttemptToConnect(addr);
	}
	else
	{
		AttemptToConnect(m_config.serverAddr);
	}
}


void DatabaseClient::BeginFrame()
{
	if (m_connection)
	{
		ProcessConnection();
	}
}


void DatabaseClient::EndFrame()
{

}


void DatabaseClient::Shutdown()
{

}


void DatabaseClient::AttemptToConnect(std::string const& addr)
{
	AttemptToConnect(NetAddress::FromString(addr));
}


void DatabaseClient::AttemptToConnect(NetAddress const& addr)
{
	TCPConnection* conn = new TCPConnection();
	if (conn->Connect(addr))
	{
		conn->SetBlocking(false);
		m_connection = conn;
	}
	else
	{
		delete conn;
	}
}


void DatabaseClient::SendCommand(std::string cmd, bool isEcho)
{
	size_t payloadSize = 1 + 2 + cmd.size() + 1;

	DataHeader header;
	header.payloadSize = ::htons((uint16_t)payloadSize);

	DataPayload payload;
	payload.isEcho = isEcho;
	payload.messageSize = ::htons((uint16_t)(cmd.size() + 1));

	m_connection->Send(&header, sizeof(header));
	m_connection->Send(&payload, sizeof(payload));
	m_connection->Send(cmd.c_str(), cmd.size() + 1);
}


void DatabaseClient::WaitUntilConnectionIsProcessed()
{
	while (ProcessConnection())
	{
		Sleep(10);
	}
}


bool DatabaseClient::ProcessConnection()
{
	size_t const bufferSize = 4096;
	char buffer[bufferSize];
	bool isEcho = true;
	if (m_connection->ReceiveFull(buffer))
	{
		char* iter = buffer;
		isEcho = *iter;
		byte byteOne = *++iter;
		byte byteTwo = *++iter;
		size_t messageSize = (byteOne << 8) | byteTwo;
		std::string command(++iter, messageSize - 1);
		if (isEcho)
		{
			g_theDevConsole->AddLine(Rgba8::YELLOW, command);
		}
		else
		{
			m_lastReceivedData = command;
		}
	}

	if (!m_connection->CheckForConnection() || !m_connection->IsConnected())
	{
		CloseConnection();
	}

	if (m_connection->isClosed())
	{
		delete m_connection;
		m_connection = nullptr;
	}

	return isEcho;
}


void DatabaseClient::CloseConnection()
{
	m_connection->Close();
	delete m_connection;
	m_connection = nullptr;
}


std::string const& DatabaseClient::GetLastReceivedData() const
{
	return m_lastReceivedData;
}


void DatabaseClient::SendLoginRequest(std::string const& user, std::string const& password)
{
	std::string command = Stringf("DBLogin user=%s password=%s", user.c_str(), password.c_str());
	SendCommand(command, false);
}


void DatabaseClient::SendViewItemFactoryRequest()
{
	std::string command = "DBGet collection=ItemFactory";
	SendCommand(command, false);
}


void DatabaseClient::AddItemToInventory(std::string const& inventoryID, std::string const& itemID)
{
	std::string command = Stringf("DBAdd inventoryID=%s itemID=%s", inventoryID.c_str(), itemID.c_str());
	SendCommand(command, false);
}


void DatabaseClient::RemoveItemFromInventory(std::string const& inventoryID, std::string const& itemID)
{
	std::string command = Stringf("DBRemove inventoryID=%s itemID=%s", inventoryID.c_str(), itemID.c_str());
	SendCommand(command, false);
}


void DatabaseClient::GetInventoryContent(std::string const& inventoryID)
{
	std::string command = Stringf("DBGetInventory inventoryID=%s", inventoryID.c_str());
	SendCommand(command, false);
}


void DatabaseClient::AddItemWorldLocation(std::string const& inventoryID, std::string const& pos)
{
	std::string command = Stringf("DBAddWorldLocation inventoryID=%s location=%s", inventoryID.c_str(), pos.c_str());
	SendCommand(command, false);
}


void DatabaseClient::RemoveItemWorldLocation(std::string const& inventoryID, std::string const& pos)
{
	std::string command = Stringf("DBRemoveWorldLocation inventoryID=%s location=%s", inventoryID.c_str(), pos.c_str());
	SendCommand(command, false);
}


void DatabaseClient::GetWorldLocation(std::string const& inventoryID)
{
	std::string command = Stringf("DBGetWorldLocation inventoryID=%s", inventoryID.c_str());
	SendCommand(command, false);
}


