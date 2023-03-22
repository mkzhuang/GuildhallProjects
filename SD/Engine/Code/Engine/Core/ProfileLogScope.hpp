#pragma once
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/DevConsole.hpp"

#include <string>

class ProfileLogScope
{
public:
	ProfileLogScope(char const* logDesc)
		: m_logDesc(logDesc)
		, m_startTime(GetCurrentTimeSeconds())
	{
	}

	~ProfileLogScope()
	{
		double duration = GetCurrentTimeSeconds() - m_startTime;
		std::string line = Stringf("%s: %.5fs", m_logDesc, duration);
		g_theDevConsole->AddLine(Rgba8::WHITE, line);
	}

private:
	char const* m_logDesc;
	double m_startTime = 0.0;
};