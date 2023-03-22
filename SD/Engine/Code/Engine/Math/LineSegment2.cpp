#include "Engine/Math/LineSegment2.hpp"

LineSegment2::LineSegment2(LineSegment2 const& copy)
	: m_start(copy.m_start)
	, m_end(copy.m_end)
{

}

LineSegment2::LineSegment2(Vec2 start, Vec2 end)
	: m_start(start)
	, m_end(end)
{

}

LineSegment2::LineSegment2(float startX, float startY, float endX, float endY)
	: m_start(Vec2(startX, startY))
	, m_end(Vec2(endX, endY))
{

}


Vec2 LineSegment2::GetCenter() const
{
	float x = 0.5f * (m_start.x + m_end.x);
	float y = 0.5f * (m_start.y + m_end.y);
	return Vec2(x, y);
}


void LineSegment2::Translate(Vec2 const& translation)
{
	m_start += translation;
	m_end += translation;
}

void LineSegment2::SetCenter(Vec2 const& newCenter)
{
	Vec2 curCenter = GetCenter();
	Vec2 translation = newCenter - curCenter;
	Translate(translation);
}

void LineSegment2::RotateAboutCenter(float const& rotationDeltaDegrees)
{
	Vec2 curCenter = GetCenter();
	Vec2 endToCenterDisplacement = m_end - curCenter;
	endToCenterDisplacement.RotateDegrees(rotationDeltaDegrees);
	m_end = curCenter + endToCenterDisplacement;
	m_start = curCenter - endToCenterDisplacement;
}


