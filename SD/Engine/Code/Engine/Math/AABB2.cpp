#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

AABB2 const AABB2::ZERO_TO_ONE(Vec2(0.f, 0.f), Vec2(1.f, 1.f));
AABB2 const AABB2::ONE_TO_ZERO(Vec2(1.f, 1.f), Vec2(0.f, 0.f));

AABB2::AABB2(AABB2 const& copy)
	: m_mins(copy.m_mins)
	, m_maxs(copy.m_maxs)
{
}


AABB2::AABB2(Vec2 mins, Vec2 maxs)
	: m_mins(mins)
	, m_maxs(maxs)
{
}


AABB2::AABB2(float minX, float minY, float maxX, float maxY)
	: m_mins(Vec2(minX, minY))
	, m_maxs(Vec2(maxX, maxY))
{
}


bool AABB2::IsPointInside(Vec2 const& point) const
{
	return (point.x > m_mins.x) && (point.x < m_maxs.x) && (point.y > m_mins.y) && (point.y < m_maxs.y);
}


Vec2 AABB2::GetCenter() const
{
	float x = 0.5f * (m_mins.x + m_maxs.x);
	float y = 0.5f * (m_mins.y + m_maxs.y);
	return Vec2(x, y);
}


Vec2 AABB2::GetDimensions() const
{
	float x = m_maxs.x - m_mins.x;
	float y = m_maxs.y - m_mins.y;
	return Vec2(x, y);
}


Vec2 AABB2::GetNearestPoint(Vec2 const& referencePoint) const
{
	float x = referencePoint.x;
	float y = referencePoint.y;
	x = Clamp(x, m_mins.x, m_maxs.x);
	y = Clamp(y, m_mins.y, m_maxs.y);
	return Vec2(x, y);
}


Vec2 AABB2::GetPointAtUV(Vec2 const& uv) const
{
	float x = Interpolate(m_mins.x, m_maxs.x, uv.x);
	float y = Interpolate(m_mins.y, m_maxs.y, uv.y);
	return Vec2(x, y);
}


Vec2 AABB2::GetUVForPoint(Vec2 const& point) const
{
	float u = GetFractionWithin(point.x, m_mins.x, m_maxs.x);
	float v = GetFractionWithin(point.y, m_mins.y, m_maxs.y);
	return Vec2(u, v);
}


AABB2 AABB2::GetBoxWithIn(AABB2 uvs) const
{
	Vec2 mins = GetPointAtUV(uvs.m_mins);
	Vec2 maxs = GetPointAtUV(uvs.m_maxs);
	return AABB2(mins, maxs);
}


void AABB2::Translate(Vec2 const& translation)
{
	m_mins += translation;
	m_maxs += translation;
}


void AABB2::SetCenter(Vec2 const& newCenter)
{
	Vec2 curCenter = GetCenter();
	Vec2 translation = newCenter - curCenter;
	Translate(translation);
}


void AABB2::SetDimensions(Vec2 const& dimensions)
{
	Vec2 center = GetCenter();
	Vec2 quarterBox = Vec2(0.5f * dimensions.x, 0.5f * dimensions.y);
	m_mins = center - quarterBox;
	m_maxs = center + quarterBox;
}


void AABB2::StretchToIncludePoint(Vec2 const& point)
{
	float x = point.x;
	float y = point.y;
	if (x < m_mins.x) m_mins.x = x;
	if (x > m_maxs.x) m_maxs.x = x;
	if (y < m_mins.y) m_mins.y = y;
	if (y > m_maxs.y) m_maxs.y = y;
}


void AABB2::AlignBoxWithin(AABB2& box, Vec2 const& alignment) const
{
	Vec2 boundingDimensions = GetDimensions();
	Vec2 fittingDimensions = box.GetDimensions();
	float horizontalPadding = boundingDimensions.x - fittingDimensions.x;
	float verticalPadding = boundingDimensions.y - fittingDimensions.y;
	float leftPadding = alignment.x * horizontalPadding;
	float bottomPadding = alignment.y * verticalPadding;

	//box.m_mins = m_mins + Vec2(leftPadding, bottomPadding);
	//box.m_maxs = box.m_mins + fittingDimensions;
	box.SetCenter(m_mins + Vec2(leftPadding, bottomPadding) + fittingDimensions * 0.5f);
}


void AABB2::SetFromText(std::string const& text)
{
	Strings tokens = SplitStringOnDelimiter(text, ';');

	if (tokens.size() == 2)
	{
		m_mins.SetFromText(tokens[0]);
		m_maxs.SetFromText(tokens[1]);
	}
	else
	{
		ERROR_AND_DIE("AABB2 set from text input incorrect.");
	}
}


