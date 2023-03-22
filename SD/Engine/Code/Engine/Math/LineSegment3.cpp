#include "Engine/Math/LineSegment3.hpp"

LineSegment3::LineSegment3(Vec3 start, Vec3 end)
	: m_start(start)
	, m_end(end)
{
}


Vec3 LineSegment3::GetCenter() const
{
	float x = 0.5f * (m_start.x + m_end.x);
	float y = 0.5f * (m_start.y + m_end.y);
	float z = 0.5f * (m_start.z + m_end.z);
	return Vec3(x, y, z);
}


void LineSegment3::Translate(Vec3 const& translation)
{
	m_start += translation;
	m_end += translation;
}


void LineSegment3::SetCenter(Vec3 const& newCenter)
{
	Vec3 curCenter = GetCenter();
	Vec3 translation = newCenter - curCenter;
	Translate(translation);
}


