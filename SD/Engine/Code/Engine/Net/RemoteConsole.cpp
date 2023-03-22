#include "Engine/Net/RemoteConsole.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EventSystem.hpp"

#include <iostream>
#include <Winsock2.h>

RemoteConsole* g_theRemoteConsole;

RemoteConsole::RemoteConsole(RemoteConsoleConfig const& config)
	: m_console(config.console)
{
}


void RemoteConsole::Startup()
{	
	SubscribeEventCallbackFunction("RCLeave", Command_RCLeave);
	SubscribeEventCallbackFunction("RCJoin", Command_RCJoin);
	SubscribeEventCallbackFunction("RCHost", Command_RCHost);
	SubscribeEventCallbackFunction("RCKick", Command_RCKick);
	SubscribeEventCallbackFunction("RC", Command_RC);
	SubscribeEventCallbackFunction("RCA", Command_RCA);
	SubscribeEventCallbackFunction("RCEcho", Command_RCEcho);
	SubscribeEventCallbackFunction("RCAEcho", Command_RCAEcho);

	m_invalidRemoteCommands.push_back("RCJoin");
	m_invalidRemoteCommands.push_back("RCHost");
	m_invalidRemoteCommands.push_back("RCKick");
	m_invalidRemoteCommands.push_back("RCLeave");
	m_invalidRemoteCommands.push_back("RC");
	m_invalidRemoteCommands.push_back("RCA");
	m_invalidRemoteCommands.push_back("QuitApp");

	NetAddress loopback = NetAddress::GetLoopBack(REMOTE_CONSOLE_PORT);
	AttemptToConnect(loopback);
	AttemptToHost(REMOTE_CONSOLE_PORT);
}


void RemoteConsole::BeginFrame()
{
	if (m_state == State::Disconnected)
	{
		NetAddress loopback = NetAddress::GetLoopBack(REMOTE_CONSOLE_PORT);
		AttemptToConnect(loopback);
		AttemptToHost(REMOTE_CONSOLE_PORT);
	}
	else
	{
		if (m_server != nullptr)
		{
			TCPConnection* conn = m_server->Accept();
			if (conn != nullptr)
			{
				bool isBlackListed = false;
				unsigned int connIP = conn->GetIP();
				for (unsigned int addr : m_blackList)
				{
					if (connIP == addr)
					{
						isBlackListed = true;
					}
				}

				if (isBlackListed)
				{
					conn->Close();
				}
				else
				{
					m_connections.push_back(conn);
					std::string connInfo = Stringf("%s is connected.", conn->GetAddressString().c_str());
					g_theDevConsole->AddLine(DevConsole::INFO_INPUT, connInfo);
				}
			}
		}

		ProcessConnections();

		if (m_state!= State::Server && m_connections.size() == 0)
		{
			m_state = State::Leave;
		}
	}
}


void RemoteConsole::EndFrame()
{

}


void RemoteConsole::Shutdown()
{

}


void RemoteConsole::AttemptToHost(uint16_t port)
{
	if (m_state != State::Disconnected && m_state != State::Leave) return;

	uint16_t servicePort = port;
	TCPServer* server = new TCPServer();
	if (server->Host(servicePort))
	{
		server->SetBlocking(false);
		m_server = server;
		m_state = State::Server;
	}
	else
	{
		NetAddress loopback = NetAddress::GetLoopBack(port);
		AttemptToConnect(loopback);
	}
}


void RemoteConsole::AttemptToConnect(std::string const& addr)
{
	AttemptToConnect(NetAddress::FromString(addr));
}


void RemoteConsole::AttemptToConnect(NetAddress const& addr)
{
	if (m_state != State::Disconnected && m_state != State::Leave) return;

	TCPConnection* conn = new TCPConnection();
	if (conn->Connect(addr))
	{
		conn->SetBlocking(false);
		m_connections.push_back(conn);
		m_state = State::Client;
	}
	else
	{
		delete conn;
		m_state = State::Disconnected;
	}
}


void RemoteConsole::SendCommandAll(std::string cmd, bool isEcho)
{
	for (int connIndex = 0; connIndex < (int)m_connections.size(); connIndex++)
	{
		SendCommand(connIndex, cmd, isEcho);
	}
}


void RemoteConsole::SendCommand(int connectionIndex, std::string cmd, bool isEcho)
{
	if (connectionIndex < 0 || connectionIndex == (int)m_connections.size())
	{
		return;
	}

	TCPConnection* conn = m_connections[connectionIndex];

	size_t payloadSize = 1 + 2 + cmd.size() + 1;

	RCHeader header;
	header.payloadSize = ::htons((uint16_t)payloadSize);

	RCPayload payload;
	payload.isEcho = isEcho;
	payload.messageSize = ::htons((uint16_t)(cmd.size() + 1));

	conn->Send(&header, sizeof(header));
	conn->Send(&payload, sizeof(payload));
	conn->Send(cmd.c_str(), cmd.size() + 1);
}


void RemoteConsole::ProcessConnections()
{
	State prevState = m_state;
	for (int connIndex = 0; connIndex < (int)m_connections.size(); connIndex++)
	{
		size_t const bufferSize = 4096;
		char buffer[bufferSize];
		TCPConnection* conn = m_connections[connIndex];
		if (conn->ReceiveFull(buffer))
		{
			char* iter = buffer;
			bool isEcho = *iter;
			byte byteOne = *++iter;
			byte byteTwo = *++iter;
			size_t messageSize = (byteOne << 8) | byteTwo;
			std::string command(++iter, messageSize - 1);
			if (isEcho)
			{
				std::string echoMessage = Stringf("%d [Echo %s]: %s", connIndex, conn->GetAddressString().c_str(), command.c_str());
				g_theDevConsole->AddLine(Rgba8::WHITE, echoMessage);
			}
			else
			{
				std::string commandKey = SplitStringOnDelimiter(command, ' ')[0];
				bool result = CheckValidCommand(commandKey);
				if (result)
				{
					g_theDevConsole->Execute(command);
					std::string echoMessage = Stringf("Executing: %s", command.c_str());
					g_theDevConsole->AddLine(Rgba8::WHITE, echoMessage);
					SendCommand(connIndex, echoMessage, true);
					if (commandKey == "help")
					{
						SendHelpCommand(connIndex);
					}
				}
				else
				{
					std::string echoMessage = Stringf("Command: %s is blocked", commandKey.c_str());
					g_theDevConsole->AddLine(Rgba8::WHITE, echoMessage);
					SendCommand(connIndex, echoMessage, true);
				}
			}

			conn->SetHistory(command);
		}

		if (!conn->CheckForConnection() || !conn->IsConnected())
		{
			conn->Close();
		}

		if (conn->isClosed() && prevState == m_state)
		{
			m_connections.erase( m_connections.begin() + connIndex);
			std::string connInfo = Stringf("%s is disconnected.", conn->GetAddressString().c_str());
			g_theDevConsole->AddLine(DevConsole::INFO_INPUT, connInfo);
			delete conn;
			connIndex--;
		}
	}
}


void RemoteConsole::CloseServer()
{
	m_server->Close();
}


void RemoteConsole::CloseConnections()
{
	for (TCPConnection* conn : m_connections)
	{
		conn->Close();
	}
	m_connections.clear();
}


void RemoteConsole::Kick(int index)
{
	TCPConnection* conn = m_connections[index];
	m_blackList.push_back(conn->GetIP());
	conn->Close();
	m_connections.erase(m_connections.begin() + index);
	delete conn;
}


void RemoteConsole::Render(AABB2 const& bounds, Renderer& renderer, BitmapFont& font) const
{
	static float cellHeight = 20.f;
	AABB2 stateBox = bounds.GetBoxWithIn(AABB2(Vec2(0.5f, 0.95f), Vec2(1.f, 1.f)));
	std::vector<Vertex_PCU> stateBoxVerts;
	AddVertsForAABB2D(stateBoxVerts, stateBox, Rgba8(200, 200, 200, 150));
	renderer.BindTexture(nullptr);
	renderer.DrawVertexArray(stateBoxVerts);

	std::vector<Vertex_PCU> stateVerts;
	std::string state = "";
	if (m_state == State::Disconnected || m_state == State::Leave)
	{
		state = m_state == State::Disconnected ? "Disconnected" : "Leave";
		font.AddVertsForTextInBox2D(stateVerts, stateBox, cellHeight, state, DevConsole::INFO_MINOR, 1.f, Vec2(0.f, 0.5f));

		renderer.BindTexture(&font.GetTexture());
		renderer.DrawVertexArray(stateVerts);
		return;
	}

	if (m_state == State::Server)
	{
		std::vector<NetAddress> locals = NetAddress::GetAllInternals(m_server->GetPort());
		state = Stringf("Server: %s", locals[0].ToString().c_str());
	}

	if (m_state == State::Client)
	{
		state = Stringf("Client: %s", m_connections[0]->GetAddressString().c_str());
	}

	font.AddVertsForTextInBox2D(stateVerts, stateBox, cellHeight, state, DevConsole::INFO_MINOR, 1.f, Vec2(0.f, 0.5f));

	renderer.BindTexture(&font.GetTexture());
	renderer.DrawVertexArray(stateVerts);

	size_t numConnections = m_connections.size();
	Vec2 connectionMins(stateBox.m_mins.x, stateBox.m_mins.y - cellHeight * static_cast<float>(numConnections));
	Vec2 connectionMaxs(stateBox.m_maxs.x, stateBox.m_mins.y);
	AABB2 connectionBox(connectionMins, connectionMaxs);
	std::vector<Vertex_PCU> connectionBoxVerts;
	AddVertsForAABB2D(connectionBoxVerts, connectionBox, Rgba8(100, 100, 100, 100));
	renderer.BindTexture(nullptr);
	renderer.DrawVertexArray(connectionBoxVerts);

	std::vector<Vertex_PCU> connectionVerts;
	float heightUV = 1.f / numConnections;
	for (int connIndex = (int)numConnections - 1; connIndex >= 0; connIndex--)
	{
		TCPConnection* conn = m_connections[connIndex];
		std::string history = conn->GetHistory();
		std::string connLine = Stringf("%d [%s] %s", connIndex, conn->GetAddressString().c_str(), history.c_str());
		float connBoxUVmin = static_cast<float>(numConnections - connIndex - 1) * heightUV;
		AABB2 connBox = connectionBox.GetBoxWithIn(AABB2(Vec2(0.f, connBoxUVmin), Vec2(1.f, connBoxUVmin + heightUV)));
		font.AddVertsForTextInBox2D(connectionVerts, connBox, cellHeight, connLine, DevConsole::INFO_INPUT, 1.f, Vec2(0.f, 0.5f));
	}

	renderer.BindTexture(&font.GetTexture());
	renderer.DrawVertexArray(connectionVerts);
}


void RemoteConsole::SendHelpCommand(int connIndex)
{
	std::vector<std::string> commands;
	g_theEventSystem->GetRegisteredEventNames(commands);

	std::string numCommands = Stringf("## Registered Commands [%d] ##", commands.size());
	SendCommand(connIndex, numCommands, true);

	for (int index = 0; index < (int)commands.size(); index++)
	{
		SendCommand(connIndex, commands[index], true);
	}
}


bool RemoteConsole::CheckValidCommand(std::string const& commandKey)
{
	for (std::string invalidKey : m_invalidRemoteCommands)
	{
		if (commandKey == invalidKey) return false;
	}
	return true;
}


State RemoteConsole::GetCurrentState() const
{
	return m_state;
}


void RemoteConsole::SetCurrentState(State state)
{
	m_state = state;
}


bool RemoteConsole::Command_RCLeave(EventArgs& args)
{
	UNUSED(args)

	if (!g_theRemoteConsole) return false;

	State state = g_theRemoteConsole->GetCurrentState();

	switch (state)
	{
		case State::Server:
			g_theRemoteConsole->CloseServer();
		case State::Client:
			g_theRemoteConsole->CloseConnections();
		case State::Disconnected:
		case State::Leave:
			g_theRemoteConsole->SetCurrentState(State::Leave);
	}

	return false;
}


bool RemoteConsole::Command_RCJoin(EventArgs& args)
{
	if (!g_theRemoteConsole) return false;

	FireEvent("RCLeave");
	std::string address = args.GetValue("addr", "");
	g_theRemoteConsole->AttemptToConnect(NetAddress::FromString(address));
	return false;
}


bool RemoteConsole::Command_RCHost(EventArgs& args)
{
	if (!g_theRemoteConsole) return false;

	FireEvent("RCLeave");
	int port = args.GetValue("port", REMOTE_CONSOLE_PORT);
	g_theRemoteConsole->AttemptToHost(static_cast<uint16_t>(port));
	return false;
}


bool RemoteConsole::Command_RCKick(EventArgs& args)
{
	if (!g_theRemoteConsole) return false;

	int index = args.GetValue("idx", 0);
	g_theRemoteConsole->Kick(index);
	return false;
}


bool RemoteConsole::Command_RC(EventArgs& args)
{
	if (!g_theRemoteConsole) return false;

	int connIndex = args.GetValue("idx", 0);
	std::string message = args.GetValue("cmd", "");
	std::string command = Stringf("%s", message.c_str());
	g_theRemoteConsole->SendCommand(connIndex, command, false);
	return false;
}


bool RemoteConsole::Command_RCA(EventArgs& args)
{
	if (!g_theRemoteConsole) return false;
	
	std::string message = args.GetValue("cmd", "");
	std::string command = Stringf("%s", message.c_str());
	g_theRemoteConsole->SendCommandAll(command, false);
	return false;
}


bool RemoteConsole::Command_RCEcho(EventArgs& args)
{
	if (!g_theRemoteConsole) return false;
	
	int connIndex = args.GetValue("idx", 0);
	std::string message = args.GetValue("msg", "");
	std::string command = Stringf("%s", message.c_str());
	g_theRemoteConsole->SendCommand(connIndex, command, true);
	return false;
}


bool RemoteConsole::Command_RCAEcho(EventArgs& args)
{
	if (!g_theRemoteConsole) return false;

	std::string message = args.GetValue("msg", "");
	std::string command = Stringf("%s", message.c_str());
	g_theRemoteConsole->SendCommandAll(command, true);
	return false;
}


