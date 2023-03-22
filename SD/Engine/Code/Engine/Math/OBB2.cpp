#include "Engine/Math/OBB2.hpp"
#include "Engine/Core/EngineCommon.hpp"

OBB2::OBB2(OBB2 const& copy)
	: m_center(copy.m_center)
	, m_iBasisNormal(copy.m_iBasisNormal)
	, m_halfDimensions(copy.m_halfDimensions)
{

}


OBB2::OBB2(Vec2 center, Vec2 iBasisNormal, Vec2 halfDimensions)
	: m_center(center)
	, m_iBasisNormal(iBasisNormal)
	, m_halfDimensions(halfDimensions)
{

}


void OBB2::GetCornerPoints(Vec2* out_fourCornerWorldPositions) const
{
	float halfXDimension = m_halfDimensions.x;
	float halfYDimension = m_halfDimensions.y;
	Vec2 m_jBasisNormal = m_iBasisNormal.GetRotated90Degrees();

	Vec2 halfIDisplacement = m_iBasisNormal * halfXDimension;
	Vec2 halfJDisplacement = m_jBasisNormal * halfYDimension;
	Vec2 bottomLeft = m_center - halfIDisplacement - halfJDisplacement;
	Vec2 bottomRight = m_center + halfIDisplacement - halfJDisplacement;
	Vec2 topRight = m_center + halfIDisplacement + halfJDisplacement;
	Vec2 topLeft = m_center - halfIDisplacement + halfJDisplacement;

	out_fourCornerWorldPositions[0] = bottomLeft;
	out_fourCornerWorldPositions[1] = bottomRight;
	out_fourCornerWorldPositions[2] = topRight;
	out_fourCornerWorldPositions[3] = topLeft;
}


Vec2 OBB2::GetLocalPosForWorldPos(Vec2 worldPos) const
{
	Vec2 displacement = worldPos - m_center;
	return Vec2(DotProduct2D(displacement, m_iBasisNormal), DotProduct2D(displacement, m_iBasisNormal.GetRotated90Degrees()));
}


Vec2 OBB2::GetWorldPosForLocalPos(Vec2 localPos) const
{
	return m_center + localPos.x * m_iBasisNormal + localPos.y * m_iBasisNormal.GetRotated90Degrees();
}


void OBB2::RotateAboutCenter(float rotationDeltaDegrees)
{
	m_iBasisNormal.RotateDegrees(rotationDeltaDegrees);
}


