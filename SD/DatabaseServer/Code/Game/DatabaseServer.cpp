#include "Game/DatabaseServer.hpp"
#include "Game/DBConn.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ProfileLogScope.hpp"

#include <iostream>
#include <Winsock2.h>
#include "mongocxx/instance.hpp"

DatabaseServer::DatabaseServer(DatabaseServerConfig const& config)
	: m_console(config.console)
{
	mongocxx::instance instance{};
	m_dbConnection = new DBConn();
}


void DatabaseServer::Startup()
{
	AttemptToHost(SERVER_PORT);
}


void DatabaseServer::BeginFrame()
{
	if (m_server != nullptr)
	{
		TCPConnection* conn = m_server->Accept();
		if (conn != nullptr)
		{
			m_connections.push_back(conn);
			m_inventoryIDs.push_back(-1);
			std::string connInfo = Stringf("[Database] %s is connected.", conn->GetAddressString().c_str());
			g_theDevConsole->AddLine(Rgba8::WHITE, connInfo);
		}
	}

	ProcessConnections();
}


void DatabaseServer::EndFrame()
{

}


void DatabaseServer::Shutdown()
{
	CloseConnections();
	CloseServer();
	m_dbConnection->ClearResult();
}


void DatabaseServer::AttemptToHost(uint16_t port)
{
	uint16_t servicePort = port;
	TCPServer* server = new TCPServer();
	if (server->Host(servicePort))
	{
		server->SetBlocking(false);
		m_server = server;
	}
}


void DatabaseServer::ProcessConnections()
{
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
				std::string echoMessage = Stringf("[Database] %d [Echo %s]: %s", connIndex, conn->GetAddressString().c_str(), command.c_str());
				g_theDevConsole->AddLine(Rgba8::WHITE, echoMessage);
			}
			else
			{
				Strings tokens;
				ParseConsoleCommand(command, tokens);
				if (tokens[0] != "DBLogin" && m_inventoryIDs[connIndex] == -1)
				{
					SendCommand(connIndex, "[Database] Invalid user, use \"DBLogin user=[user] password=[password]\" to login", true);
				}
				else
				{
					bool result = ProcessCommand(command, connIndex);
					std::string echoMessage = Stringf("[Database] Executing: %s", command.c_str());
					g_theDevConsole->AddLine(Rgba8::WHITE, echoMessage);
					if (!result)
					{
						SendCommand(connIndex, "[Database] Request failed, please modify and try again", true);
					}
					else
					{
						std::string message = Stringf("[Database] %s request received by server", tokens[0].c_str());
						SendCommand(connIndex, message, true);
					}
				}
			}
		}

		if (!conn->CheckForConnection() || !conn->IsConnected())
		{
			conn->Close();
		}

		if (conn->isClosed())
		{
			m_connections.erase(m_connections.begin() + connIndex);
			m_inventoryIDs.erase(m_inventoryIDs.begin() + connIndex);
			m_dbConnection->ClearResult();
			std::string connInfo = Stringf("%s is disconnected.", conn->GetAddressString().c_str());
			g_theDevConsole->AddLine(DevConsole::INFO_INPUT, connInfo);
			delete conn;
			connIndex--;
		}
	}
}


void DatabaseServer::SendCommand(int connectionIndex, std::string cmd, bool isEcho)
{
	if (connectionIndex < 0 || connectionIndex == (int)m_connections.size())
	{
		return;
	}

	TCPConnection* conn = m_connections[connectionIndex];

	size_t payloadSize = 1 + 2 + cmd.size() + 1;

	DataHeader header;
	header.payloadSize = ::htons((uint16_t)payloadSize);

	DataPayload payload;
	payload.isEcho = isEcho;
	payload.messageSize = ::htons((uint16_t)(cmd.size() + 1));

	conn->Send(&header, sizeof(header));
	conn->Send(&payload, sizeof(payload));
	conn->Send(cmd.c_str(), cmd.size() + 1);
}


void DatabaseServer::CloseServer()
{
	m_server->Close();
	m_server = nullptr;
}


void DatabaseServer::CloseConnections()
{
	for (TCPConnection* conn : m_connections)
	{
		conn->Close();
	}
	m_connections.clear();
}


void DatabaseServer::Render(AABB2 const& bounds, Renderer& renderer, BitmapFont& font) const
{
	static float cellHeight = 20.f;
	AABB2 addressBox = bounds.GetBoxWithIn(AABB2(Vec2(0.5f, 0.95f), Vec2(1.f, 1.f)));
	std::vector<Vertex_PCU> addressBoxVerts;
	AddVertsForAABB2D(addressBoxVerts, addressBox, Rgba8(200, 200, 200, 150));
	renderer.BindTexture(nullptr);
	renderer.DrawVertexArray(addressBoxVerts);

	std::vector<Vertex_PCU> addressVerts;
	std::vector<NetAddress> locals = NetAddress::GetAllInternals(m_server->GetPort());
	std::string address = Stringf("Server: %s", locals[0].ToString().c_str());
	font.AddVertsForTextInBox2D(addressVerts, addressBox, cellHeight, address, DevConsole::INFO_MINOR, 1.f, Vec2(0.f, 0.5f));
	renderer.BindTexture(&font.GetTexture());
	renderer.DrawVertexArray(addressVerts);

	size_t numConnections = m_connections.size();
	Vec2 connectionMins(addressBox.m_mins.x, addressBox.m_mins.y - cellHeight * static_cast<float>(numConnections));
	Vec2 connectionMaxs(addressBox.m_maxs.x, addressBox.m_mins.y);
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


bool DatabaseServer::ProcessCommand(std::string const& command, int connectionIndex)
{
	Strings tokens;
	ParseConsoleCommand(command, tokens);
	std::string commandName = tokens[0];
	EventArgs argumentPairs;
	for (int tokenIndex = 1; tokenIndex < int(tokens.size()); tokenIndex++)
	{
		Strings argumentPair = SplitStringOnFirstDelimiter(tokens[tokenIndex], '=');
		if (argumentPair.size() == 2)
		{
			argumentPairs.SetValue(argumentPair[0], argumentPair[1]);
		}
	}

	if (commandName == "DBLogin")
	{
		return Query_Login(argumentPairs, connectionIndex);
	}
	else if (commandName == "DBGet")
	{
		return Query_Get(argumentPairs, connectionIndex);
	}
	else if (commandName == "DBValidate")
	{
		return Query_Validate(argumentPairs, connectionIndex);
	}
	else if (commandName == "DBGetInventory")
	{
		return Query_GetInventory(argumentPairs, connectionIndex);
	}
	else if (commandName == "DBGetWorldLocation")
	{
		return Query_GetWorldLocation(argumentPairs, connectionIndex);
	}
	else if (commandName == "DBAddWorldLocation")
	{
		return Query_AddWorldLocation(argumentPairs, connectionIndex);
	}
	else if (commandName == "DBRemoveWorldLocation")
	{
		return Query_RemoveWorldLocation(argumentPairs, connectionIndex);
	}
	else if (commandName == "DBAdd")
	{
		return Query_Add(argumentPairs, connectionIndex);
	}
	else if (commandName == "DBRemove")
	{
		return Query_Remove(argumentPairs, connectionIndex);
	}

	return false;
}


bool DatabaseServer::Query_Login(EventArgs& args, int connectionIndex)
{
	ProfileLogScope profiler("Query Login");

	m_dbConnection->ExecuteQuery(QueryType::Login, args);
	std::string line = m_dbConnection->Result();

	if (!line.empty())
	{
		int inventoryID = ParseInventoryID(line);
		m_inventoryIDs[connectionIndex] = inventoryID;
		SendCommand(connectionIndex, std::to_string(inventoryID), false);
		return true;
	}
	return false;
}

bool DatabaseServer::Query_Get(EventArgs& args, int connectionIndex)
{
	ProfileLogScope profiler("Query Get");

	m_dbConnection->ExecuteQuery(QueryType::Get, args);
	std::string line = m_dbConnection->Result();

	if (!line.empty())
	{
		SendCommand(connectionIndex, line, false);
		return true;
	}
	return false;
}


bool DatabaseServer::Query_Validate(EventArgs& args, int connectionIndex)
{
	UNUSED(args)
	UNUSED(connectionIndex)
	return true;
}


bool DatabaseServer::Query_GetInventory(EventArgs& args, int connectionIndex)
{
	ProfileLogScope profiler("Query Get Inventory");

	m_dbConnection->ExecuteQuery(QueryType::GetInventory, args);
	std::string line = m_dbConnection->Result();

	if (!line.empty())
	{
		std::string inventoryInfo;
		ParseInventoryInfo(line, inventoryInfo);
		SendCommand(connectionIndex, inventoryInfo, false);
		return true;
	}
	return false;
}


bool DatabaseServer::Query_GetWorldLocation(EventArgs& args, int connectionIndex)
{
	ProfileLogScope profiler("Query Get World Location");

	m_dbConnection->ExecuteQuery(QueryType::GetWorldLocation, args);
	std::string line = m_dbConnection->Result();

	if (!line.empty())
	{
		std::string inventoryInfo;
		ParseWorldLocationInfo(line, inventoryInfo);
		SendCommand(connectionIndex, inventoryInfo, false);
		return true;
	}
	return false;
}


bool DatabaseServer::Query_AddWorldLocation(EventArgs& args, int connectionIndex)
{
	UNUSED(connectionIndex)
	ProfileLogScope profiler("Query AddWorldLocation");

	m_dbConnection->ExecuteQuery(QueryType::AddWorldLocation, args);
	std::string line = m_dbConnection->Result();

	if (std::atoi(line.c_str()) == 0)
	{
		return false;
	}
	return true;
}


bool DatabaseServer::Query_RemoveWorldLocation(EventArgs& args, int connectionIndex)
{
	UNUSED(connectionIndex)
	ProfileLogScope profiler("Query Remove World Location");

	m_dbConnection->ExecuteQuery(QueryType::RemoveWorldLocation, args);
	std::string line = m_dbConnection->Result();

	if (std::atoi(line.c_str()) == 0)
	{
		return false;
	}
	return true;
}


bool DatabaseServer::Query_Add(EventArgs& args, int connectionIndex)
{
	UNUSED(connectionIndex)
	ProfileLogScope profiler("Query Add");

	m_dbConnection->ExecuteQuery(QueryType::Add, args);
	std::string line = m_dbConnection->Result();

	if (std::atoi(line.c_str()) == 0)
	{
		return false;
	}
	return true;
}


bool DatabaseServer::Query_Remove(EventArgs& args, int connectionIndex)
{
	UNUSED(connectionIndex)
	ProfileLogScope profiler("Query Remove");

	m_dbConnection->ExecuteQuery(QueryType::Remove, args);
	std::string line = m_dbConnection->Result();

	if (std::atoi(line.c_str()) == 0)
	{
		return false;
	}
	return true;
}


int DatabaseServer::ParseInventoryID(std::string const& jsonString)
{
	size_t idBegin = jsonString.find("inventory") + 14;
	std::string id;
	for (size_t charIndex = idBegin; charIndex < jsonString.size(); charIndex++)
	{
		char const& charAtIndex = jsonString[charIndex];
		if (charAtIndex != '\"')
		{
			id += charAtIndex;
		}
		else
		{
			break;
		}
	}

	return std::stoi(id);
}


void DatabaseServer::ParseInventoryInfo(std::string const& jsonString, std::string& info)
{
	size_t infoBegin = jsonString.find("items") + 10;
	for (size_t charIndex = infoBegin; charIndex < jsonString.size(); charIndex++)
	{
		char const& charAtIndex = jsonString[charIndex];
		if (charAtIndex != ']')
		{
			info += charAtIndex;
		}
		else
		{
			break;
		}
	}
	info = info.substr(0, info.size() - 1);
	if (info.find_first_not_of(' ') == std::string::npos)
	{
		info = "";
	}
}


void DatabaseServer::ParseWorldLocationInfo(std::string const& jsonString, std::string& info)
{
	size_t infoBegin = jsonString.find("items") + 10;
	for (size_t charIndex = infoBegin; charIndex < jsonString.size(); charIndex++)
	{
		char const& charAtIndex = jsonString[charIndex];
		if (charAtIndex != ']')
		{
			info += charAtIndex;
		}
		else
		{
			break;
		}
	}
	info = info.substr(0, info.size() - 1);
	if (info.find_first_not_of(' ') == std::string::npos)
	{
		info = "";
	}
}


