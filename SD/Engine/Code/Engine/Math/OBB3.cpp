#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"

OBB3::OBB3(Vec3 center, EulerAngles orientation, Vec3 halfDimensions)
	: m_center(center)
	, m_orientation(orientation)
	, m_halfDimensions(halfDimensions)
{
}


void OBB3::GetCornerPoints(Vec3* out_cornerWorldPositions) const
{
	float halfXDimension = m_halfDimensions.x;
	float halfYDimension = m_halfDimensions.y;
	float halfZDimension = m_halfDimensions.z;
	Mat44 orientationMatrix = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 iBasisNormal = orientationMatrix.GetIBasis3D();
	Vec3 jBasisNormal = orientationMatrix.GetJBasis3D();
	Vec3 kBasisNormal = orientationMatrix.GetKBasis3D();
	Vec3 halfIDisplacement = iBasisNormal * halfXDimension;
	Vec3 halfJDisplacement = jBasisNormal * halfYDimension;
	Vec3 halfKDisplacement = kBasisNormal * halfZDimension;

	Vec3 nearBL = m_center - halfIDisplacement - halfJDisplacement - halfKDisplacement;
	Vec3 nearBR = m_center + halfIDisplacement - halfJDisplacement - halfKDisplacement;
	Vec3 nearTR = m_center + halfIDisplacement - halfJDisplacement + halfKDisplacement;
	Vec3 nearTL = m_center - halfIDisplacement - halfJDisplacement + halfKDisplacement;
	Vec3 farBL	= m_center - halfIDisplacement + halfJDisplacement - halfKDisplacement;
	Vec3 farBR	= m_center + halfIDisplacement + halfJDisplacement - halfKDisplacement;
	Vec3 farTR	= m_center + halfIDisplacement + halfJDisplacement + halfKDisplacement;
	Vec3 farTL	= m_center - halfIDisplacement + halfJDisplacement + halfKDisplacement;

	out_cornerWorldPositions[0] = nearBL;
	out_cornerWorldPositions[1] = nearBR;
	out_cornerWorldPositions[2] = nearTR;
	out_cornerWorldPositions[3] = nearTL;
	out_cornerWorldPositions[4] = farBL;
	out_cornerWorldPositions[5] = farBR;
	out_cornerWorldPositions[6] = farTR;
	out_cornerWorldPositions[7] = farTL;
}


Vec3 OBB3::GetLocalPosForWorldPos(Vec3 worldPos) const
{
	Mat44 orientationMatrix = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 iBasisNormal = orientationMatrix.GetIBasis3D();
	Vec3 jBasisNormal = orientationMatrix.GetJBasis3D();
	Vec3 kBasisNormal = orientationMatrix.GetKBasis3D();

	Vec3 displacement = worldPos - m_center;
	return Vec3(DotProduct3D(displacement, iBasisNormal), DotProduct3D(displacement, jBasisNormal), DotProduct3D(displacement, kBasisNormal));
}


Vec3 OBB3::GetWorldPosForLocalPos(Vec3 localPos) const
{
	Mat44 orientationMatrix = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 iBasisNormal = orientationMatrix.GetIBasis3D();
	Vec3 jBasisNormal = orientationMatrix.GetJBasis3D();
	Vec3 kBasisNormal = orientationMatrix.GetKBasis3D();

	return m_center + localPos.x * iBasisNormal + localPos.y * jBasisNormal + localPos.z * kBasisNormal;
}


