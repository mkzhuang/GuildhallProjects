#include "Engine/Net/NetSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

NetSystem::NetSystem(NetSystemConfig const& config)
	: m_config(config)
{
}


NetSystem::~NetSystem()
{

}


void NetSystem::Startup()
{
	WORD version = MAKEWORD(2, 2);
	WSADATA data;

	int error = ::WSAStartup(version, &data);
	GUARANTEE_OR_DIE(error == 0, "Failed to initialize NetSystem");
}


void NetSystem::BeginFrame()
{

}


void NetSystem::EndFrame()
{

}


void NetSystem::Shutdown()
{
	::WSACleanup();
}


