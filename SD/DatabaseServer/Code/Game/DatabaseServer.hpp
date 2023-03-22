#pragma once
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Net/TCPConnection.hpp"
#include "Engine/Net/TCPServer.hpp"

#include <stdint.h>
#include <vector>
#include <map>

class Renderer;
class TCPServer;
class DBConn;

constexpr uint16_t SERVER_PORT = 3121;

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

struct DatabaseServerConfig
{
	DevConsole* console = nullptr;
};

class DatabaseServer
{
public:
	DatabaseServer(DatabaseServerConfig const& config);
	~DatabaseServer() {}

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void AttemptToHost(uint16_t port);
	void ProcessConnections();
	void SendCommand(int connectionIndex, std::string cmd, bool isEcho);
	void CloseServer();
	void CloseConnections();
	void Render(AABB2 const& bounds, Renderer& renderer, BitmapFont& font) const;

	bool ProcessCommand(std::string const& command, int connectionIndex = 0);
	bool Query_Login(EventArgs& args, int connectionIndex = 0);
	bool Query_Get(EventArgs& args, int connectionIndex = 0);
	bool Query_Validate(EventArgs& args, int connectionIndex = 0);
	bool Query_GetInventory(EventArgs& args, int connectionIndex = 0);
	bool Query_GetWorldLocation(EventArgs& args, int connectionIndex = 0);
	bool Query_AddWorldLocation(EventArgs& args, int connectionIndex = 0);
	bool Query_RemoveWorldLocation(EventArgs& args, int connectionIndex = 0);
	bool Query_Add(EventArgs& args, int connectionIndex = 0);
	bool Query_Remove(EventArgs& args, int connectionIndex = 0);

private:
	int ParseInventoryID(std::string const& jsonString);
	void ParseInventoryInfo(std::string const& jsonString, std::string& info);
	void ParseWorldLocationInfo(std::string const& jsonString, std::string& info);

protected:
	DevConsole* m_console = nullptr;
	std::vector<TCPConnection*> m_connections;
	std::vector<int> m_inventoryIDs;
	TCPServer* m_server = nullptr;
	DBConn* m_dbConnection = nullptr;
};


