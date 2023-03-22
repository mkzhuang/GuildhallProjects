#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"

Clock g_systemClock;

Clock::Clock()
{
	if (this != &g_systemClock)
	{
		m_parent = &g_systemClock;
		g_systemClock.AddChild(this);
	}
}


Clock::Clock(Clock& parent)
	: m_parent(&parent)
{
	parent.AddChild(this);
}


Clock::~Clock()
{
	//reparent child clock to their grandparent
	if (this == &g_systemClock) return;
	for (int index = 0; index < m_children.size(); index++)
	{
		m_children[index]->SetParent(*m_parent);
	}
}


void Clock::SetParent(Clock& parent)
{
	m_parent = &parent;
	m_parent->AddChild(this);
}


void Clock::Pause()
{
	m_isPaused = true;
}


void Clock::Unpause()
{
	m_isPaused = false;
}


void Clock::TogglePause()
{
	m_isPaused = !m_isPaused;
}


void Clock::StepFrame()
{
	Unpause();
	m_pauseAfterFrame = true;
}


void Clock::SetTimeDilation(double dilationAmount)
{
	m_timeDilation = dilationAmount;
}


double Clock::GetDeltaTime() const
{
	return m_deltaTime;
}


double Clock::GetTotalTime() const
{
	return m_totalTime;
}


size_t Clock::GetFrameCount() const
{
	return m_frameCount;
}


bool Clock::IsPaused() const
{
	return m_isPaused;
}


double Clock::GetTimeDilation() const
{
	return m_timeDilation;
}


void Clock::SystemBeginFrame()
{
	g_systemClock.Tick();
}


Clock& Clock::GetSystemClock()
{
	return g_systemClock;
}


void Clock::Tick()
{
	double currentTime = GetCurrentTimeSeconds();
	double deltaTime = currentTime - m_lastUpdateTime;
	m_lastUpdateTime = currentTime;
	if (deltaTime > 0.1) deltaTime = 0.1;
	Advance(deltaTime);
}


void Clock::Advance(double deltaTimeSeconds)
{
	if (m_isPaused) deltaTimeSeconds = 0.0;
	deltaTimeSeconds *= m_timeDilation;

	m_deltaTime = deltaTimeSeconds;
	m_totalTime += deltaTimeSeconds;
	if (!m_isPaused) m_frameCount++;

	for (int index = 0; index < m_children.size(); index++)
	{
		if (m_children[index])
		{
			m_children[index]->Advance(deltaTimeSeconds);
		}
	}

	if (m_pauseAfterFrame)
	{
		Pause();
		m_pauseAfterFrame = false;
	}
}


void Clock::AddChild(Clock* childClock)
{
	for (int index = 0; index < m_children.size(); index++)
	{
		if (!m_children[index])
		{
			m_children[index] = childClock;
			return;
		}
	}
	m_children.push_back(childClock);
}


void Clock::RemoveChild(Clock* childClock)
{
	for (int index = 0; index < m_children.size(); index++)
	{
		if (m_children[index] && m_children[index] == childClock)
		{
			m_children[index] = nullptr;
			return;
		}
	}
}


