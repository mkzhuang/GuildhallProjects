#pragma once
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Net/TCPConnection.hpp"
#include "Engine/Net/TCPServer.hpp"

#include <stdint.h>
#include <vector>
#include <map>

class Renderer;
class TCPServer;

struct DataHeader
{
	uint16_t payloadSize;
};

#pragma pack(push, 1)
struct DataPayload
{
	char isEcho;
	uint16_t messageSize;
};
#pragma pack(pop)

struct DatabaseClientConfig
{
	DevConsole* console = nullptr;
	std::string serverAddr = "";
};

class DatabaseClient
{
public:
	DatabaseClient(DatabaseClientConfig const& config);
	~DatabaseClient() {}

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void AttemptToConnect(std::string const& addr);
	void AttemptToConnect(NetAddress const& addr);
	void SendCommand(std::string cmd, bool isEcho);
	void WaitUntilConnectionIsProcessed();
	bool ProcessConnection();
	void CloseConnection();
	std::string const& GetLastReceivedData() const;

	void SendLoginRequest(std::string const& user, std::string const& password);
	void SendViewItemFactoryRequest();
	void AddItemToInventory(std::string const& inventoryID, std::string const& itemID);
	void RemoveItemFromInventory(std::string const& inventoryID, std::string const& itemID);
	void GetInventoryContent(std::string const& inventoryID);
	void AddItemWorldLocation(std::string const& inventoryID, std::string const& pos);
	void RemoveItemWorldLocation(std::string const& inventoryID, std::string const& pos);
	void GetWorldLocation(std::string const& inventoryID);

protected:
	DatabaseClientConfig m_config;
	DevConsole* m_console = nullptr;
	TCPConnection* m_connection = nullptr;
	std::string m_lastReceivedData = "";
};