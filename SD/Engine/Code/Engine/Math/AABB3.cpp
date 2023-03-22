#include "Engine/Math/AABB3.hpp"

AABB3::AABB3(AABB3 const& copy)
	: m_mins(copy.m_mins)
	, m_maxs(copy.m_maxs)
{
}


AABB3::AABB3(Vec3 mins, Vec3 maxs)
	: m_mins(mins)
	, m_maxs(maxs)
{
}


AABB3::AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
	: m_mins(Vec3(minX, minY, minZ))
	, m_maxs(Vec3(maxX, maxY, maxZ))
{
}


AABB3 const AABB3::ZERO_TO_ONE(Vec3::ZERO, Vec3::ONE);


void AABB3::GetCorners(Vec3* corners) const
{
	Vec3 nearBL = m_mins;
	Vec3 nearBR = Vec3(m_maxs.x, m_mins.y, m_mins.z);
	Vec3 nearTL = Vec3(m_mins.x, m_mins.y, m_maxs.z);
	Vec3 nearTR = Vec3(m_maxs.x, m_mins.y, m_maxs.z);
	Vec3 farBL = Vec3(m_mins.x, m_maxs.y, m_mins.z);
	Vec3 farBR = Vec3(m_maxs.x, m_maxs.y, m_mins.z);
	Vec3 farTL = Vec3(m_mins.x, m_maxs.y, m_maxs.z);
	Vec3 farTR = m_maxs;
	corners[0] = nearBL;
	corners[1] = nearBR;
	corners[2] = nearTR;
	corners[3] = nearTL;
	corners[4] = farBL;
	corners[5] = farBR;
	corners[6] = farTR;
	corners[7] = farTL;
}


bool AABB3::IsPointInside(Vec3 const& point) const
{
	return (point.x > m_mins.x) && (point.x < m_maxs.x) && (point.y > m_mins.y) && (point.y < m_maxs.y) && (point.z > m_mins.z) && (point.z < m_maxs.z);
}


Vec3 AABB3::GetCenter() const
{
	float x = 0.5f * (m_mins.x + m_maxs.x);
	float y = 0.5f * (m_mins.y + m_maxs.y);
	float z = 0.5f * (m_mins.z + m_maxs.z);
	return Vec3(x, y, z);
}


Vec3 AABB3::GetCenterAtMinZ() const
{
	float x = 0.5f * (m_mins.x + m_maxs.x);
	float y = 0.5f * (m_mins.y + m_maxs.y);
	return Vec3(x, y, m_mins.z);
}


Vec3 AABB3::GetHalfDimensions() const
{
	float x = 0.5f * (m_maxs.x - m_mins.x);
	float y = 0.5f * (m_maxs.y - m_mins.y);
	float z = 0.5f * (m_maxs.z - m_mins.z);
	return Vec3(x, y, z);
}


void AABB3::Translate(Vec3 const& translation)
{
	m_mins += translation;
	m_maxs += translation;
}


void AABB3::SetCenter(Vec3 const& newCenter)
{
	Vec3 curCenter = GetCenter();
	Vec3 translation = newCenter - curCenter;
	Translate(translation);
}


void AABB3::SetCenterAtMinZ(Vec3 const& newCenter)
{
	Vec3 curCenter = GetCenter();
	Vec3 translation = Vec3(newCenter.x, newCenter.y, newCenter.z + GetHalfDimensions().z) - curCenter;
	Translate(translation);
}


