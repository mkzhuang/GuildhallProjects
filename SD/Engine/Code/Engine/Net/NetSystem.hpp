#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>

#include "Engine/Net/NetCommon.hpp"

struct NetSystemConfig
{
};

class NetSystem
{
public:
	NetSystem(NetSystemConfig const& config);
	~NetSystem();
	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

protected:
	NetSystemConfig m_config;
};