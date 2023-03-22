#pragma once
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Net/TCPConnection.hpp"
#include "Engine/Net/TCPServer.hpp"

#include <stdint.h>
#include <vector>
#include <map>

enum class State
{
	Disconnected,
	Client,
	Server,
	Leave,
};

class Renderer;
class TCPServer;

constexpr uint16_t REMOTE_CONSOLE_PORT = 3121;

struct RCHeader
{
	uint16_t payloadSize;
};

#pragma pack(push, 1)
struct RCPayload
{
	char isEcho;
	uint16_t messageSize;
};
#pragma pack(pop)

struct RemoteConsoleConfig
{
	DevConsole* console = nullptr;
};

class RemoteConsole
{
public:
	RemoteConsole(RemoteConsoleConfig const& config);
	~RemoteConsole() {}

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void AttemptToHost(uint16_t port);
	void AttemptToConnect(std::string const& addr);
	void AttemptToConnect(NetAddress const& addr);
	void SendCommandAll(std::string cmd, bool isEcho);
	void SendCommand(int connectionIndex, std::string cmd, bool isEcho);
	void ProcessConnections();
	void CloseServer();
	void CloseConnections();
	void Kick(int index);
	void Render(AABB2 const& bounds, Renderer& renderer, BitmapFont& font) const;

	void SendHelpCommand(int connIndex);
	bool CheckValidCommand(std::string const& commandKey);
	State GetCurrentState() const;
	void SetCurrentState(State state);

	static bool Command_RCLeave(EventArgs& args);
	static bool Command_RCJoin(EventArgs& args);
	static bool Command_RCHost(EventArgs& args);
	static bool Command_RCKick(EventArgs& args);
	static bool Command_RC(EventArgs& args);
	static bool Command_RCA(EventArgs& args);
	static bool Command_RCEcho(EventArgs& args);
	static bool Command_RCAEcho(EventArgs& args);

protected:
	DevConsole* m_console = nullptr;
	std::vector<TCPConnection*> m_connections;
	TCPServer* m_server = nullptr;
	State m_state = State::Disconnected;
	std::vector<unsigned int> m_blackList;
	std::vector<std::string> m_invalidRemoteCommands;
};