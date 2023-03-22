#include "Engine/Core/Stopwatch.hpp"

Stopwatch::Stopwatch()
	: m_clock(m_clock = &Clock::GetSystemClock())
{
}


Stopwatch::Stopwatch(double duration)
	: m_clock(m_clock = &Clock::GetSystemClock())
	, m_duration(duration)
{
}


Stopwatch::Stopwatch(const Clock* clock, double duration)
	: m_clock(clock)
	, m_duration(duration)
{
	m_startTime = m_clock->GetTotalTime();
}


void Stopwatch::Start(double duration)
{
	m_duration = duration;
}


void Stopwatch::Start(const Clock* clock, double duration)
{
	m_clock = clock;
	m_startTime = m_clock->GetTotalTime();
	m_duration = duration;
}


void Stopwatch::Restart()
{
	m_startTime = m_clock->GetTotalTime();
}


void Stopwatch::Stop()
{
	m_duration = 0.0;
}


void Stopwatch::Pause()
{
	m_startTime = -(m_clock->GetTotalTime() - m_startTime);
}


void Stopwatch::Resume()
{
	m_startTime = m_clock->GetTotalTime() + m_startTime;
}


double Stopwatch::GetElapsedTime() const
{
	if (IsStopped())
	{
		return 0.0;
	}

	if (IsPaused())
	{
		return -m_startTime;
	}

	return m_clock->GetTotalTime() - m_startTime;
}


float Stopwatch::GetElapsedFraction() const
{
	return static_cast<float>(GetElapsedTime() / m_duration);
}


bool Stopwatch::IsStopped() const
{
	return m_duration == 0.0;
}


bool Stopwatch::IsPaused() const
{
	return m_startTime < 0.0;
}


bool Stopwatch::HasDurationElapsed() const
{
	return GetElapsedTime() >= m_duration;
}


bool Stopwatch::CheckDurationElapsedAndDecrement()
{
	if (IsStopped() || !HasDurationElapsed())
	{
		return false;
	}

	m_startTime += m_duration;
	return true;
}


