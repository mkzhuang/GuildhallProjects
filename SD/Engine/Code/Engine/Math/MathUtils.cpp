#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math//IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/LineSegment3.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Capsule3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/FloatRange.hpp"

#include <math.h>
#define UNUSED(x) (void)(x);

RaycastResult2D::RaycastResult2D(bool didImpact, Vec2 impactPosition, float impactDistance, Vec2 impactSurfaceNormal, Vec2 startPosition, Vec2 forwardNormal, float maxDistance)
	: m_didImpact(didImpact)
	, m_impactPosition(impactPosition)
	, m_impactDistance(impactDistance)
	, m_impactSurfaceNormal(impactSurfaceNormal)
	, m_startPosition(startPosition)
	, m_forwardNormal(forwardNormal)
	, m_maxDistance(maxDistance)
{
}


RaycastResult3D::RaycastResult3D(bool didImpact, Vec3 impactPosition, float impactDistance, Vec3 impactSurtaceNormal, Vec3 startPosition, Vec3 forwardNormal, float maxDistance)
	: m_didImpact(didImpact)
	, m_impactPosition(impactPosition)
	, m_impactDistance(impactDistance)
	, m_impactSurfaceNormal(impactSurtaceNormal)
	, m_startPosition(startPosition)
	, m_forwardNormal(forwardNormal)
	, m_maxDistance(maxDistance)
{
}


constexpr float PI = 3.14159265358979323846f;

float ConvertDegreesToRadians(float degrees)
{
	return degrees * (PI / 180.f);
}


float ConvertRadiansToDegrees(float radians)
{
	return radians * (180.f / PI);
}


float CosDegrees(float degrees)
{
	return cosf(ConvertDegreesToRadians(degrees));
}


float SinDegrees(float degrees)
{
	return sinf(ConvertDegreesToRadians(degrees));
}


float Atan2Degrees(float y, float x)
{
	return ConvertRadiansToDegrees(atan2f(y, x));
}


float GetShortestAngularDispDegrees(float fromDegrees, float toDegrees)
{
	float displacementDegrees = toDegrees - fromDegrees;
	while (displacementDegrees > 180.f) displacementDegrees -= 360.f;
	while (displacementDegrees < -180.f) displacementDegrees += 360.f;
	return displacementDegrees;
}


float GetTurnedTowardDegrees(float fromDegrees, float toDegrees, float maxDeltaDegrees)
{
	float displacementDegrees = GetShortestAngularDispDegrees(fromDegrees, toDegrees);
	if (fabsf(displacementDegrees) <= maxDeltaDegrees) return toDegrees;
	float resultDegrees = displacementDegrees > 0 ? fromDegrees + maxDeltaDegrees : fromDegrees - maxDeltaDegrees;
	return resultDegrees;
}


Vec4 GetQuaternionsForAngleAxis(Vec3 const& axis)
{
	Vec3 direction = axis.GetNormalized();
	float magnitude = axis.GetLength();
	Vec3 qv = sinf(0.5f * magnitude) * direction;
	float qw = cosf(0.5f * magnitude);
	return Vec4(qv.x, qv.y, qv.z, qw);
}


Vec4 MultiplyQuaternions(Vec4 const& q1, Vec4 const& q2)
{
	float x = q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x;
	float y = -q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y;
	float z = q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z;
	float w = -q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w;
	return Vec4(x, y, z, w);
}


Vec3 GetAngleAxisForQuaternions(Vec4 const& quaternions)
{
	float ww = quaternions.w * quaternions.w;
	float w = 2.f * acosf(quaternions.w);
	float x = quaternions.x / sqrtf(1 - ww);
	float y = quaternions.y / sqrtf(1 - ww);
	float z = quaternions.z / sqrtf(1 - ww);
	return Vec3(x, y, z) * w;
}


float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB)
{
	float deltaX = positionB.x - positionA.x;
	float deltaY = positionB.y - positionA.y;
	return sqrtf((deltaX * deltaX) + (deltaY * deltaY));
}


float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB)
{
	float deltaX = positionB.x - positionA.x;
	float deltaY = positionB.y - positionA.y;
	return (deltaX * deltaX) + (deltaY * deltaY);
}


float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float deltaX = positionB.x - positionA.x;
	float deltaY = positionB.y - positionA.y;
	float deltaZ = positionB.z - positionA.z;
	return sqrtf((deltaX * deltaX) + (deltaY * deltaY) + (deltaZ * deltaZ));
}


float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float deltaX = positionB.x - positionA.x;
	float deltaY = positionB.y - positionA.y;
	float deltaZ = positionB.z - positionA.z;
	return (deltaX * deltaX) + (deltaY * deltaY) + (deltaZ * deltaZ);
}


float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float deltaX = positionB.x - positionA.x;
	float deltaY = positionB.y - positionA.y;
	return sqrtf((deltaX * deltaX) + (deltaY * deltaY));
}


float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float deltaX = positionB.x - positionA.x;
	float deltaY = positionB.y - positionA.y;
	return (deltaX * deltaX) + (deltaY * deltaY);
}


int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB)
{
	return (pointB - pointA).GetTaxicabLength();
}


float DotProduct2D(Vec2 const& a, Vec2 const& b)
{
	return (a.x * b.x) + (a.y * b.y);
}


float DotProduct3D(Vec3 const& a, Vec3 const& b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}


float DotProduct4D(Vec4 const& a, Vec4 const& b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}


float CrossProduct2D(Vec2 const& a, Vec2 const& b)
{
	return (a.x * b.y) - (a.y * b.x);
}


Vec3 const CrossProduct3D(Vec3 const& a, Vec3 const& b)
{
	float x = (a.y * b.z) - (a.z * b.y);
	float y = (a.z * b.x) - (a.x * b.z);
	float z = (a.x * b.y) - (a.y * b.x);
	return Vec3(x, y, z);
}


float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	return DotProduct2D(vectorToProject, vectorToProjectOnto.GetNormalized());
}


Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	float projectedLength = GetProjectedLength2D(vectorToProject, vectorToProjectOnto);
	Vec2 projectedOntoNormalized = vectorToProjectOnto.GetNormalized();
	return projectedOntoNormalized * projectedLength;
}


float GetProjectedLength3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto)
{
	return DotProduct3D(vectorToProject, vectorToProjectOnto.GetNormalized());
}


Vec3 const GetProjectedOnto3D(Vec3 const& vectorToProject, Vec3 const& vectorProjectOnto)
{
	float projectedLength = GetProjectedLength3D(vectorToProject, vectorProjectOnto);
	Vec3 projectedOntoNormalized = vectorProjectOnto.GetNormalized();
	return projectedOntoNormalized * projectedLength;
}


float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b)
{
	float dotProduct = Clamp(DotProduct2D(a.GetNormalized(), b.GetNormalized()), -1.f, 1.f);
	return ConvertRadiansToDegrees(acosf(dotProduct));
}


RaycastResult2D const RaycastVsDisc2D(Vec2 const& startPos, Vec2 const& forwardNormal, float maxDistance, Vec2 const& discCenter, float discRadius)
{
	Vec2 leftNormal = forwardNormal.GetRotated90Degrees();
	Vec2 displacementToDiscCenter = discCenter - startPos;
	float jDistanceFromDiscCenter = DotProduct2D(displacementToDiscCenter, leftNormal);
	if (jDistanceFromDiscCenter > discRadius || jDistanceFromDiscCenter < -discRadius) // miss from left or right
	{
		return RaycastResult2D(false, Vec2::ZERO, 0.f, Vec2::ZERO, startPos, forwardNormal, maxDistance);
	}

	float iDistanceFromDiscCenter = DotProduct2D(displacementToDiscCenter, forwardNormal);
	if (iDistanceFromDiscCenter < -discRadius || iDistanceFromDiscCenter > maxDistance + discRadius) // miss from front or back
	{
		return RaycastResult2D(false, Vec2::ZERO, 0.f, Vec2::ZERO, startPos, forwardNormal, maxDistance);
	}

	if (IsPointInsideDisc2D(startPos, discCenter, discRadius)) //ray start inside the disc
	{
		return RaycastResult2D(true, startPos, 0.f, -forwardNormal, startPos, forwardNormal, maxDistance);
	}

	float impactDistance = iDistanceFromDiscCenter - sqrtf(discRadius * discRadius - jDistanceFromDiscCenter * jDistanceFromDiscCenter);

	if (impactDistance < 0 || impactDistance > maxDistance) // impact before ray start or after ray ended
	{
		return RaycastResult2D(false, Vec2::ZERO, 0.f, Vec2::ZERO, startPos, forwardNormal, maxDistance);
	}

	Vec2 impactPosition = startPos + (forwardNormal * impactDistance);
	Vec2 impactNormal = (impactPosition - discCenter).GetNormalized();

	return RaycastResult2D(true, impactPosition, impactDistance, impactNormal, startPos, forwardNormal, maxDistance);
}


RaycastResult2D const RaycastVsAABB2D(Vec2 const& startPos, Vec2 const& forwardNormal, float maxDistance, AABB2 const& box)
{
	float impactLengthOnMinX = (box.m_mins.x - startPos.x) / forwardNormal.x;
	float impactLengthOnMaxX = (box.m_maxs.x - startPos.x) / forwardNormal.x;
	float impactLengthOnMinY = (box.m_mins.y - startPos.y) / forwardNormal.y;
	float impactLengthOnMaxY = (box.m_maxs.y - startPos.y) / forwardNormal.y;

	FloatRange xRange = (impactLengthOnMinX <= impactLengthOnMaxX) ? FloatRange(impactLengthOnMinX, impactLengthOnMaxX) : FloatRange(impactLengthOnMaxX, impactLengthOnMinX);
	FloatRange yRange = (impactLengthOnMinY <= impactLengthOnMaxY) ? FloatRange(impactLengthOnMinY, impactLengthOnMaxY) : FloatRange(impactLengthOnMaxY, impactLengthOnMinY);
	FloatRange overlap = xRange.GetOverlappingRangeWith(yRange);

	if (!overlap.IsOverlappingWith(FloatRange(0.f, maxDistance)) || overlap == FloatRange::ZERO)
	{
		return RaycastResult2D(false, Vec2::ZERO, 0.f, Vec2::ZERO, startPos, forwardNormal, maxDistance);
	}

	if (box.IsPointInside(startPos))
	{
		return RaycastResult2D(true, startPos, 0.f, -forwardNormal, startPos, forwardNormal, maxDistance);
	}

	float impactLength = overlap.m_min > 0.f ? overlap.m_min : overlap.m_max;
	Vec2 impactPos = startPos + forwardNormal * impactLength;
	Vec2 impactSurfaceNormal = Vec2::ZERO;
	if (impactLength == impactLengthOnMinX) impactSurfaceNormal = Vec2::WEST;
	if (impactLength == impactLengthOnMaxX) impactSurfaceNormal = Vec2::EAST;
	if (impactLength == impactLengthOnMinY) impactSurfaceNormal = Vec2::SOUTH;
	if (impactLength == impactLengthOnMaxY) impactSurfaceNormal = Vec2::NORTH;
	return RaycastResult2D(true, impactPos, impactLength, impactSurfaceNormal, startPos, forwardNormal, maxDistance);
}


RaycastResult2D const RaycastVsOBB2D(Vec2 const& startPos, Vec2 const& forwardNormal, float maxDistance, OBB2 const& box)
{
	Vec2 localStartPos = box.GetLocalPosForWorldPos(startPos);
	Vec2 boxJBasisNormal = box.m_iBasisNormal.GetRotated90Degrees();
	Vec2 localForwardNormal = Vec2(GetProjectedLength2D(forwardNormal, box.m_iBasisNormal), GetProjectedLength2D(forwardNormal, boxJBasisNormal));
	AABB2 localBox(-box.m_halfDimensions, box.m_halfDimensions);

	float impactLengthOnMinX = (localBox.m_mins.x - localStartPos.x) / localForwardNormal.x;
	float impactLengthOnMaxX = (localBox.m_maxs.x - localStartPos.x) / localForwardNormal.x;
	float impactLengthOnMinY = (localBox.m_mins.y - localStartPos.y) / localForwardNormal.y;
	float impactLengthOnMaxY = (localBox.m_maxs.y - localStartPos.y) / localForwardNormal.y;

	FloatRange xRange = (impactLengthOnMinX <= impactLengthOnMaxX) ? FloatRange(impactLengthOnMinX, impactLengthOnMaxX) : FloatRange(impactLengthOnMaxX, impactLengthOnMinX);
	FloatRange yRange = (impactLengthOnMinY <= impactLengthOnMaxY) ? FloatRange(impactLengthOnMinY, impactLengthOnMaxY) : FloatRange(impactLengthOnMaxY, impactLengthOnMinY);
	FloatRange overlap = xRange.GetOverlappingRangeWith(yRange);

	if (!overlap.IsOverlappingWith(FloatRange(0.f, maxDistance)) || overlap == FloatRange::ZERO)
	{
		return RaycastResult2D(false, Vec2::ZERO, 0.f, Vec2::ZERO, startPos, forwardNormal, maxDistance);
	}

	if (localBox.IsPointInside(localStartPos))
	{
		return RaycastResult2D(true, startPos, 0.f, -forwardNormal, startPos, forwardNormal, maxDistance);
	}

	float impactLength = overlap.m_min > 0.f ? overlap.m_min : overlap.m_max;
	Vec2 impactPos = startPos + forwardNormal * impactLength;
	Vec2 impactSurfaceNormal = Vec2::ZERO;
	if (impactLength == impactLengthOnMinX) impactSurfaceNormal = Vec2::WEST;
	if (impactLength == impactLengthOnMaxX) impactSurfaceNormal = Vec2::EAST;
	if (impactLength == impactLengthOnMinY) impactSurfaceNormal = Vec2::SOUTH;
	if (impactLength == impactLengthOnMaxY) impactSurfaceNormal = Vec2::NORTH;
	impactSurfaceNormal = impactSurfaceNormal.x * box.m_iBasisNormal + impactSurfaceNormal.y * boxJBasisNormal;
	return RaycastResult2D(true, impactPos, impactLength, impactSurfaceNormal, startPos, forwardNormal, maxDistance);
}


RaycastResult2D const RaycastVsLineSegment2D(Vec2 const& startPos, Vec2 const& forwardNormal, float maxDistance, LineSegment2 const& line)
{
	Vec2 jNormal = forwardNormal.GetRotated90Degrees();
	Vec2 rayStartToLineStartDisplacement = line.m_start - startPos;
	Vec2 rayStartToLineEndDisplacement = line.m_end - startPos;
	float jDistanceFromRayStartToLineStart = DotProduct2D(rayStartToLineStartDisplacement, jNormal);
	float jDistanceFromRayStartToLineEnd = DotProduct2D(rayStartToLineEndDisplacement, jNormal);

	if (jDistanceFromRayStartToLineStart * jDistanceFromRayStartToLineEnd >= 0.f)
	{
		return RaycastResult2D(false, Vec2::ZERO, 0.f, Vec2::ZERO, startPos, forwardNormal, maxDistance);
	}

	float iDistanceFromRayStartToLineStart = DotProduct2D(rayStartToLineStartDisplacement, forwardNormal);
	float iDistanceFromRayStartToLineEnd = DotProduct2D(rayStartToLineEndDisplacement, forwardNormal);
	FloatRange rayRange(0.f, maxDistance);
	FloatRange iRange(iDistanceFromRayStartToLineStart, iDistanceFromRayStartToLineEnd);

	if (!iRange.IsOverlappingWith(rayRange))
	{
		return RaycastResult2D(false, Vec2::ZERO, 0.f, Vec2::ZERO, startPos, forwardNormal, maxDistance);
	}

	float impactFraction = jDistanceFromRayStartToLineStart / (jDistanceFromRayStartToLineStart - jDistanceFromRayStartToLineEnd);
	Vec2 lineDisplacement = line.m_end - line.m_start;
	Vec2 impactPosition = line.m_start + impactFraction * lineDisplacement;
	Vec2 impactDisplacement = impactPosition - startPos;
	float impactDistance = DotProduct2D(impactDisplacement, forwardNormal);

	if (!rayRange.IsOnRange(impactDistance))
	{
		return RaycastResult2D(false, Vec2::ZERO, 0.f, Vec2::ZERO, startPos, forwardNormal, maxDistance);
	}

	Vec2 impactNormal = lineDisplacement.GetRotated90Degrees().GetNormalized();
	if (jDistanceFromRayStartToLineStart > 0.f) impactNormal = -impactNormal;

	return RaycastResult2D(true, impactPosition, impactDistance, impactNormal, startPos, forwardNormal, maxDistance);
}


RaycastResult3D const RaycastVsSphere3D(Vec3 const& startPos, Vec3 const& forwardNormal, float maxDistance, Vec3 const& sphereCenter, float sphereRadius)
{
	Vec3 displacementToSphereCenter = sphereCenter - startPos;
	float forwardDistanceFromSphereCenter = DotProduct3D(displacementToSphereCenter, forwardNormal);
	if (forwardDistanceFromSphereCenter < -sphereRadius || forwardDistanceFromSphereCenter > maxDistance + sphereRadius) // miss from front or back
	{
		return RaycastResult3D(false, Vec3::ZERO, 0.f, Vec3::ZERO, startPos, forwardNormal, maxDistance);
	}

	float displacementToSphereCenterLength = displacementToSphereCenter.GetLength();
	float notForwardDistanceFromSphereCenter = sqrtf((displacementToSphereCenterLength * displacementToSphereCenterLength) - (forwardDistanceFromSphereCenter * forwardDistanceFromSphereCenter));
	if (notForwardDistanceFromSphereCenter > sphereRadius || notForwardDistanceFromSphereCenter < -sphereRadius) // miss from left or right
	{
		return RaycastResult3D(false, Vec3::ZERO, 0.f, Vec3::ZERO, startPos, forwardNormal, maxDistance);
	}

	if (IsPointInsideSphere3D(startPos, sphereCenter, sphereRadius)) //ray start inside the sphere
	{
		return RaycastResult3D(true, startPos, 0.f, -forwardNormal, startPos, forwardNormal, maxDistance);
	}

	float impactDistance = forwardDistanceFromSphereCenter - sqrtf(sphereRadius * sphereRadius - notForwardDistanceFromSphereCenter * notForwardDistanceFromSphereCenter);

	if (impactDistance < 0 || impactDistance > maxDistance) // impact before ray start or after ray ended
	{
		return RaycastResult3D(false, Vec3::ZERO, 0.f, Vec3::ZERO, startPos, forwardNormal, maxDistance);
	}

	Vec3 impactPosition = startPos + (forwardNormal * impactDistance);
	Vec3 impactNormal = (impactPosition - sphereCenter).GetNormalized();

	return RaycastResult3D(true, impactPosition, impactDistance, impactNormal, startPos, forwardNormal, maxDistance);
}


RaycastResult3D const RaycastVsAABB3D(Vec3 const& startPos, Vec3 const& forwardNormal, float maxDistance, AABB3 const& box)
{
	float impactLengthOnMinX = (box.m_mins.x - startPos.x) / forwardNormal.x;
	float impactLengthOnMaxX = (box.m_maxs.x - startPos.x) / forwardNormal.x;
	float impactLengthOnMinY = (box.m_mins.y - startPos.y) / forwardNormal.y;
	float impactLengthOnMaxY = (box.m_maxs.y - startPos.y) / forwardNormal.y;
	float impactLengthOnMinZ = (box.m_mins.z - startPos.z) / forwardNormal.z;
	float impactLengthOnMaxZ = (box.m_maxs.z - startPos.z) / forwardNormal.z;

	FloatRange xRange = (impactLengthOnMinX <= impactLengthOnMaxX) ? FloatRange(impactLengthOnMinX, impactLengthOnMaxX) : FloatRange(impactLengthOnMaxX, impactLengthOnMinX);
	FloatRange yRange = (impactLengthOnMinY <= impactLengthOnMaxY) ? FloatRange(impactLengthOnMinY, impactLengthOnMaxY) : FloatRange(impactLengthOnMaxY, impactLengthOnMinY);
	FloatRange overlapXY = xRange.GetOverlappingRangeWith(yRange);
	if (!overlapXY.IsOverlappingWith(FloatRange(0.f, maxDistance)) || overlapXY == FloatRange::ZERO)
	{
		return RaycastResult3D(false, Vec3::ZERO, 0.f, Vec3::ZERO, startPos, forwardNormal, maxDistance);
	}

	FloatRange zRange = (impactLengthOnMinZ <= impactLengthOnMaxZ) ? FloatRange(impactLengthOnMinZ, impactLengthOnMaxZ) : FloatRange(impactLengthOnMaxZ, impactLengthOnMinZ);
	FloatRange overlapXYZ = overlapXY.GetOverlappingRangeWith(zRange);
	if (!overlapXYZ.IsOverlappingWith(FloatRange(0.f, maxDistance)) || overlapXYZ == FloatRange::ZERO)
	{
		return RaycastResult3D(false, Vec3::ZERO, 0.f, Vec3::ZERO, startPos, forwardNormal, maxDistance);
	}

	if (box.IsPointInside(startPos))
	{
		return RaycastResult3D(true, startPos, 0.f, -forwardNormal, startPos, forwardNormal, maxDistance);
	}

	float impactLength = overlapXYZ.m_min > 0.f ? overlapXYZ.m_min : overlapXYZ.m_max;
	Vec3 impactPos = startPos + forwardNormal * impactLength;
	Vec3 impactSurfaceNormal = Vec3::ZERO;
	if (impactPos.x == box.m_mins.x) impactSurfaceNormal = Vec3::BACKWARD;
	if (impactPos.x == box.m_maxs.x) impactSurfaceNormal = Vec3::FORWARD;
	if (impactPos.y == box.m_mins.y) impactSurfaceNormal = Vec3::LEFT;
	if (impactPos.y == box.m_maxs.y) impactSurfaceNormal = Vec3::RIGHT;
	if (impactPos.z == box.m_mins.z) impactSurfaceNormal = Vec3::DOWN;
	if (impactPos.z == box.m_maxs.z) impactSurfaceNormal = Vec3::UP;
	return RaycastResult3D(true, impactPos, impactLength, impactSurfaceNormal, startPos, forwardNormal, maxDistance);
}


RaycastResult3D const RaycastVsZCylinder3D(Vec3 const& startPos, Vec3 const& forwardNormal, float maxDistance, Vec3 const& cylinderCenterBottom, float cylinderRadius, float cylinderHeight)
{
	Vec3 endPos = startPos + forwardNormal * maxDistance;
	Vec2 startPosXY(startPos.x, startPos.y);
	Vec2 endPosXY(endPos.x, endPos.y);
	float xyMaxDistance = (endPosXY - startPosXY).GetLength();
	float zMaxDistance = endPos.z - startPos.z;
	Vec2 forwardNormalXY = Vec2(forwardNormal.x, forwardNormal.y).GetNormalized();
	Vec2 cylinderCenterXY(cylinderCenterBottom.x, cylinderCenterBottom.y);
	// if 2d ray intersect with 2d disc
	RaycastResult2D discRaycastResult = RaycastVsDisc2D(startPosXY, forwardNormalXY, xyMaxDistance, cylinderCenterXY, cylinderRadius);
	if (!discRaycastResult.m_didImpact)
	{
		return RaycastResult3D(false, Vec3::ZERO, 0.f, Vec3::ZERO, startPos, forwardNormal, maxDistance);
	}
	
	// calculate the impact position on side of the cylinder
	float cylinderBottomZ = cylinderCenterBottom.z;
	float cylinderTopZ = cylinderBottomZ + cylinderHeight;
	float impactZDistance = (discRaycastResult.m_impactDistance * zMaxDistance) / xyMaxDistance;
	Vec3 impactPosOnSide(discRaycastResult.m_impactPosition.x, discRaycastResult.m_impactPosition.y, startPos.z + impactZDistance);

	// if ray start inside the cylinder
	if (discRaycastResult.m_impactPosition == startPosXY && startPos.z > cylinderBottomZ && startPos.z < cylinderTopZ)
	{
		return RaycastResult3D(true, startPos, 0.f, -forwardNormal, startPos, forwardNormal, maxDistance);
	}

	Vec3 displacement = endPos - startPos;
	// if ray cast down
	if (impactPosOnSide.z > cylinderTopZ && displacement.z < 0.f)
	{
		float impactTopZDistance = cylinderTopZ - startPos.z;
		float impactDistance = (maxDistance * impactTopZDistance) / zMaxDistance;

		if (impactDistance < 0 || impactDistance > maxDistance)
		{
			return RaycastResult3D(false, Vec3::ZERO, 0.f, Vec3::ZERO, startPos, forwardNormal, maxDistance); // miss
		}

		Vec3 impactPos = startPos + forwardNormal * impactDistance;
		if (IsPointInsideDisc2D(Vec2(impactPos.x, impactPos.y), cylinderCenterXY, cylinderRadius))
		{
			return RaycastResult3D(true, impactPos, impactDistance, Vec3(0.f, 0.f, 1.f), startPos, forwardNormal, maxDistance);
		}
	}
	
	// if ray cast up
	if (impactPosOnSide.z < cylinderBottomZ && displacement.z > 0.f)
	{
		float impactBottomZDistance = cylinderBottomZ - startPos.z;
		float impactDistance = (maxDistance * impactBottomZDistance) / zMaxDistance;

		if (impactDistance < 0 || impactDistance > maxDistance)
		{
			return RaycastResult3D(false, Vec3::ZERO, 0.f, Vec3::ZERO, startPos, forwardNormal, maxDistance); // miss
		}

		Vec3 impactPos = startPos + forwardNormal * impactDistance;
		if (IsPointInsideDisc2D(Vec2(impactPos.x, impactPos.y), cylinderCenterXY, cylinderRadius))
		{
			return RaycastResult3D(true, impactPos, impactDistance, Vec3(0.f, 0.f, -1.f), startPos, forwardNormal, maxDistance);
		}
	}

	//ToDo: why this needs to check one more time?
	if (impactPosOnSide.z > cylinderTopZ || impactPosOnSide.z < cylinderBottomZ)
	{
		return RaycastResult3D(false, Vec3::ZERO, 0.f, Vec3::ZERO, startPos, forwardNormal, maxDistance); // miss
	}

	float impactDistance = (impactPosOnSide - startPos).GetLength();
	Vec2 impactSurfaceNormalXY = (discRaycastResult.m_impactPosition - cylinderCenterXY).GetNormalized();
	Vec3 impactSurfaceNormal(impactSurfaceNormalXY.x, impactSurfaceNormalXY.y, 0.f);
	return RaycastResult3D(true, impactPosOnSide, impactDistance, impactSurfaceNormal, startPos, forwardNormal, maxDistance);
}


bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius)
{
	return GetDistanceSquared2D(point, discCenter) < (discRadius * discRadius);
}


bool IsPointInsideSphere3D(Vec3 const& point, Vec3 const& sphereCenter, float sphereRadius)
{
	return GetDistanceSquared3D(point, sphereCenter) < (sphereRadius * sphereRadius);
}


bool IsPointInsideZCylinder3D(Vec3 const& point, Vec3 const& cylinderBottomCenter, float cylinderRadius, float cylinderHeight)
{
	float pointZ = point.z;
	float startZ = cylinderBottomCenter.z;
	float endZ = startZ + cylinderHeight;
	Vec2 pointXYLocation(point.x, point.y);
	Vec2 cylinderXYCenter(cylinderBottomCenter.x, cylinderBottomCenter.y);
	return IsPointInsideDisc2D(pointXYLocation, cylinderXYCenter, cylinderRadius) && (pointZ > startZ) && (pointZ < endZ);
}


bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box)
{
	return box.IsPointInside(point);
}


bool IsPointInsideAABB3D(Vec3 const& point, AABB3 const& box)
{
	return box.IsPointInside(point);
}


bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule)
{
	Vec2 nearestPoint = GetNearestPointOnLineSegement2D(point, capsule.m_bone);
	return GetDistanceSquared2D(point, nearestPoint) < capsule.m_radius * capsule.m_radius;
}


bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientedBox)
{
	Vec2 displacement = point - orientedBox.m_center;
	float iDistance = GetProjectedLength2D(displacement, orientedBox.m_iBasisNormal);
	float jDistance = GetProjectedLength2D(displacement, orientedBox.m_iBasisNormal.GetRotated90Degrees());
	float halfWidth = orientedBox.m_halfDimensions.x;
	float halfHeight = orientedBox.m_halfDimensions.y;
	return (iDistance > -halfWidth && iDistance < halfWidth) && (jDistance > -halfHeight && jDistance < halfHeight);
}


bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius)
{
	if (!IsPointInsideDisc2D(point, sectorTip, sectorRadius)) return false;
	float angleToPoint = GetShortestAngularDispDegrees(sectorForwardDegrees, (point - sectorTip).GetOrientationDegrees());
	return angleToPoint > -0.5f * sectorApertureDegrees && angleToPoint < 0.5f * sectorApertureDegrees;
}


bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius)
{
	if (!IsPointInsideDisc2D(point, sectorTip, sectorRadius)) return false;
	float angleBetween = GetAngleDegreesBetweenVectors2D(sectorForwardNormal, point - sectorTip);
	return angleBetween < 0.5f * sectorApertureDegrees;
}


bool DoDiscsOverlap2D(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB)
{
	float squaredDistance = GetDistanceSquared2D(centerA, centerB);
	float squaredCombinedLength = (radiusA + radiusB) * (radiusA + radiusB);
	return squaredDistance < squaredCombinedLength;
}


bool DoAABB2sOverlap2D(AABB2 const& boxA, AABB2 const& boxB)
{
	FloatRange ax(boxA.m_mins.x, boxA.m_maxs.x);
	FloatRange bx(boxB.m_mins.x, boxB.m_maxs.x);
	if (!ax.IsOverlappingWith(bx)) return false;

	FloatRange ay(boxA.m_mins.y, boxA.m_maxs.y);
	FloatRange by(boxB.m_mins.y, boxB.m_mins.y);
	if (!ay.IsOverlappingWith(by)) return false;

	return true;
}


bool DoesAABB2OverlapWithDisc2D(AABB2 const& box, Vec2 const& discCenter, float discRadius)
{
	Vec2 nearestPointOnAABB2 = GetNearestPointOnAABB2D(discCenter, box);
	return IsPointInsideDisc2D(nearestPointOnAABB2, discCenter, discRadius);
}


bool DoesOBB2OverlapWithDisc2D(OBB2 const& box, Vec2 const& discCenter, float discRadius)
{
	Vec2 nearestPointOnOBB2 = GetNearestPointOnOBB2D(discCenter, box);
	return IsPointInsideDisc2D(nearestPointOnOBB2, discCenter, discRadius);
}


bool DoSpheresOverlap3D(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB)
{
	float squaredDistance = GetDistanceSquared3D(centerA, centerB);
	float squaredCombinedLength = (radiusA + radiusB) * (radiusA + radiusB);
	return squaredDistance < squaredCombinedLength;
}


bool DoAABB3sOverlap3D(AABB3 const& boxA, AABB3 const& boxB)
{
	FloatRange ax(boxA.m_mins.x, boxA.m_maxs.x);
	FloatRange bx(boxB.m_mins.x, boxB.m_maxs.x);
	if (!ax.IsOverlappingWith(bx)) return false;

	FloatRange ay(boxA.m_mins.y, boxA.m_maxs.y);
	FloatRange by(boxB.m_mins.y, boxB.m_maxs.y);
	if (!ay.IsOverlappingWith(by)) return false;

	FloatRange az(boxA.m_mins.z, boxA.m_maxs.z);
	FloatRange bz(boxB.m_mins.z, boxB.m_maxs.z);
	if (!az.IsOverlappingWith(bz)) return false;

	return true;
}


bool DoZCylindersOverlap3D(Vec3 const& bottomCenterA, float radiusA, float heightA, Vec3 const& bottomCenterB, float radiusB, float heightB)
{
	if (!DoDiscsOverlap2D(Vec2(bottomCenterA.x, bottomCenterA.y), radiusA, Vec2(bottomCenterB.x, bottomCenterB.y), radiusB)) return false;

	FloatRange az(bottomCenterA.z, bottomCenterA.z + heightA);
	FloatRange bz(bottomCenterB.z, bottomCenterB.z + heightB);
	if (!az.IsOverlappingWith(bz)) return false;

	return true;
}


bool DoesSphereOverlapWithAABB3D(Vec3 const& sphereCenter, float sphereRadius, AABB3 const& box)
{
	Vec3 nearestPointOnBox = GetNearestPointOnAABB3D(sphereCenter, box);
	return IsPointInsideSphere3D(nearestPointOnBox, sphereCenter, sphereRadius);
}


bool DoesSphereOverlapWithOBB3(Vec3 const& sphereCenter, float sphereRadius, OBB3 const& box)
{
	Vec3 nearestPointOnBox = GetNearestPointOnOBB3D(sphereCenter, box);
	return IsPointInsideSphere3D(nearestPointOnBox, sphereCenter, sphereRadius);
}


bool DoesSphereOverlapWithZCylinder3D(Vec3 const& sphereCenter, float sphereRadius, Vec3 const& cylinderBottomCenter, float cylinderRadius, float cylinderHeight)
{
	Vec3 nearestPointOnZCylinder = GetNearestPointOnZCylinder3D(sphereCenter, cylinderBottomCenter, cylinderRadius, cylinderHeight);
	return IsPointInsideSphere3D(nearestPointOnZCylinder, sphereCenter, sphereRadius);
}


bool DoesAABB3OverlapWithZCylinder3D(AABB3 const& box, Vec3 const& cylinderBottomCenter, float cylinderRadius, float cylinderHeight)
{
	AABB2 boxXY(Vec2(box.m_mins.x, box.m_mins.y), Vec2(box.m_maxs.x, box.m_maxs.y));
	if (!DoesAABB2OverlapWithDisc2D(boxXY, Vec2(cylinderBottomCenter.x, cylinderBottomCenter.y), cylinderRadius)) return false;

	FloatRange boxZ(box.m_mins.z, box.m_maxs.z);
	FloatRange cylinderZ(cylinderBottomCenter.z, cylinderBottomCenter.z + cylinderHeight);
	if (!boxZ.IsOverlappingWith(cylinderZ)) return false;

	return true;
}


Vec2 const GetNearestPointOnDisc2D(Vec2 const& referencePos, Vec2 const& discCenter, float discRadius)
{
	Vec2 displacement = (referencePos - discCenter).GetClamped(discRadius);
	return discCenter + displacement;
}


Vec2 const GetNearestPointOnAABB2D(Vec2 const& referencePos, AABB2 const& box)
{
	return box.GetNearestPoint(referencePos);
}


Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePos, LineSegment2 const& infiniteLine)
{
	return GetNearestPointOnLineSegement2D(referencePos, infiniteLine);
}


Vec2 const GetNearestPointOnLineSegement2D(Vec2 const& referencePos, LineSegment2 const& lineSegment)
{
	Vec2 lineDisplacement = lineSegment.m_end - lineSegment.m_start;
	Vec2 pointToLineEndDisplacement = referencePos - lineSegment.m_end;
	if (DotProduct2D(lineDisplacement, pointToLineEndDisplacement) >= 0) return lineSegment.m_end;
	Vec2 pointToLineStartDisplacement = referencePos - lineSegment.m_start;
	if (DotProduct2D(lineDisplacement, pointToLineStartDisplacement) <= 0) return lineSegment.m_start;
	return lineSegment.m_start + GetProjectedOnto2D(pointToLineStartDisplacement, lineDisplacement);
}


Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePos, Capsule2 const& capsule)
{
	Vec2 nearestPointOnBone = GetNearestPointOnLineSegement2D(referencePos, capsule.m_bone);
	Vec2 displacement = (referencePos - nearestPointOnBone).GetClamped(capsule.m_radius);
	return nearestPointOnBone + displacement;
}


Vec2 const GetNearestPointOnOBB2D(Vec2 const& referencePos, OBB2 const& orientedBox)
{
	Vec2 displacement = referencePos - orientedBox.m_center;
	float halfWidth = orientedBox.m_halfDimensions.x;
	float halfHeight = orientedBox.m_halfDimensions.y;
	float iDistance = Clamp(GetProjectedLength2D(displacement, orientedBox.m_iBasisNormal), -halfWidth, halfWidth);
	float jDistance = Clamp(GetProjectedLength2D(displacement, orientedBox.m_iBasisNormal.GetRotated90Degrees()), -halfHeight, halfHeight);
	return orientedBox.m_center + iDistance * orientedBox.m_iBasisNormal + jDistance * orientedBox.m_iBasisNormal.GetRotated90Degrees();
}


Vec3 const GetNearestPointOnSphere3D(Vec3 const& referencePos, Vec3 const& sphereCenter, float sphereRadius)
{
	Vec3 displacement = (referencePos - sphereCenter).GetClamped(sphereRadius);
	return sphereCenter + displacement;
}


Vec3 const GetNearestPointOnAABB3D(Vec3 const& referencePos, AABB3 const& box)
{
	float x = Clamp(referencePos.x, box.m_mins.x, box.m_maxs.x);
	float y = Clamp(referencePos.y, box.m_mins.y, box.m_maxs.y);
	float z = Clamp(referencePos.z, box.m_mins.z, box.m_maxs.z);
	return Vec3(x, y, z);
}


Vec3 const GetNearestPointOnOBB3D(Vec3 const& referencePos, OBB3 const& box)
{
	Vec3 displacement = referencePos - box.m_center;
	float halfXDimension = box.m_halfDimensions.x;
	float halfYDimension = box.m_halfDimensions.y;
	float halfZDimension = box.m_halfDimensions.z;

	Vec3 localPos = box.GetLocalPosForWorldPos(referencePos);
	localPos.x = Clamp(localPos.x, -halfXDimension, halfXDimension);
	localPos.y = Clamp(localPos.y, -halfYDimension, halfYDimension);
	localPos.z = Clamp(localPos.z, -halfZDimension, halfZDimension);

	return box.GetWorldPosForLocalPos(localPos);
}


Vec3 const GetNearestPointOnLineSegment3D(Vec3 const& referencePos, LineSegment3 const& lineSegment)
{
	Vec3 lineDisplacement = lineSegment.m_end - lineSegment.m_start;
	Vec3 pointToLineEndDisplacement = referencePos - lineSegment.m_end;
	if (DotProduct3D(lineDisplacement, pointToLineEndDisplacement) >= 0) return lineSegment.m_end;
	Vec3 pointToLineStartDisplacement = referencePos - lineSegment.m_start;
	if (DotProduct3D(lineDisplacement, pointToLineStartDisplacement) <= 0) return lineSegment.m_start;
	return lineSegment.m_start + GetProjectedOnto3D(pointToLineStartDisplacement, lineDisplacement);
}


Vec3 const GetNearestPointOnCapsule3D(Vec3 const& referencePos, Capsule3 const& capsule)
{
	Vec3 nearestPointOnBone = GetNearestPointOnLineSegment3D(referencePos, capsule.m_bone);
	Vec3 displacement = (referencePos - nearestPointOnBone).GetClamped(capsule.m_radius);
	return nearestPointOnBone + displacement;
}


Vec3 const GetNearestPointOnZCylinder3D(Vec3 const& referencePos, Vec3 const& cylinderBottomCenter, float cylinderRadius, float cylinderHeight)
{
	Vec2 nearnestPoint2D = GetNearestPointOnDisc2D(Vec2(referencePos.x, referencePos.y), Vec2(cylinderBottomCenter.x, cylinderBottomCenter.y), cylinderRadius);
	float z = Clamp(referencePos.z, cylinderBottomCenter.z, cylinderBottomCenter.z + cylinderHeight);
	return Vec3(nearnestPoint2D.x, nearnestPoint2D.y, z);
}


bool PushDiscOutOfPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint)
{
	if (!IsPointInsideDisc2D(fixedPoint, mobileDiscCenter, discRadius)) return false;
	float distance = GetDistance2D(mobileDiscCenter, fixedPoint);
	float overlapDepth = discRadius - distance;
	Vec2 pushDisplacement = mobileDiscCenter - fixedPoint;
	if (pushDisplacement.GetLengthSquared() == 0.f) pushDisplacement = Vec2::NORTH;
	pushDisplacement.SetLength(overlapDepth);
	mobileDiscCenter += pushDisplacement;
	return true;
}


bool PushDiscOutOfDisc2D(Vec2& mobileDiscCenter, float mobileRadius, Vec2 const& fixedDiscCenter, float fixedRadius)
{
	float totalRadius = mobileRadius + fixedRadius;
	if (GetDistanceSquared2D(mobileDiscCenter, fixedDiscCenter) > (totalRadius * totalRadius)) return false;
	Vec2 displacement = fixedDiscCenter - mobileDiscCenter;
	displacement.SetLength(totalRadius - displacement.GetLength());
	mobileDiscCenter -= displacement;
	return true;
}


bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius)
{
	float totalRadius = aRadius + bRadius;
	if (GetDistanceSquared2D(aCenter, bCenter) > (totalRadius * totalRadius)) return false;
	Vec2 dispalcement = bCenter - aCenter;
	dispalcement.SetLength(totalRadius - dispalcement.GetLength());
	aCenter -= 0.5f * dispalcement;
	bCenter += 0.5f * dispalcement;
	return true;
}


bool PushDiscOutOfAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox)
{
	// #Todo: handle disc center inside box
	Vec2 nearestPoint = fixedBox.GetNearestPoint(mobileDiscCenter);
	if (!IsPointInsideDisc2D(nearestPoint, mobileDiscCenter, discRadius)) return false;
	PushDiscOutOfPoint2D(mobileDiscCenter, discRadius, nearestPoint);
	return true;
}


bool PushDiscOutOfOBB2D(Vec2& mobileDiscCenter, float discRadius, OBB2 const& fixedBox)
{
	Vec2 nearestPoint = GetNearestPointOnOBB2D(mobileDiscCenter, fixedBox);
	if (!IsPointInsideDisc2D(nearestPoint, mobileDiscCenter, discRadius)) return false;
	PushDiscOutOfPoint2D(mobileDiscCenter, discRadius, nearestPoint);
	return true;
}


bool PushDiscOutOfCapsule2D(Vec2& mobileDiscCenter, float discRadius, Capsule2 const& capsule)
{
	Vec2 nearestPoint = GetNearestPointOnCapsule2D(mobileDiscCenter, capsule);
	if (!IsPointInsideDisc2D(nearestPoint, mobileDiscCenter, discRadius)) return false;
	PushDiscOutOfPoint2D(mobileDiscCenter, discRadius, nearestPoint);
	return true;
}


Vec3 const PushAABB3OutOfAABB3D(AABB3& mobileBox, AABB3 const& fixedBox)
{
	if (!DoAABB3sOverlap3D(mobileBox, fixedBox)) return Vec3::ONE;
	Vec3 displacement = fixedBox.GetCenter() - mobileBox.GetCenter();
	Vec3 totalHalfDimension = fixedBox.GetHalfDimensions() + mobileBox.GetHalfDimensions();
	float xDistance = totalHalfDimension.x - fabsf(displacement.x);
	float yDistance = totalHalfDimension.y - fabsf(displacement.y);
	float zDistance = totalHalfDimension.z - fabsf(displacement.z);
	if (xDistance <= yDistance && xDistance <= zDistance)
	{
		float direction = displacement.x > 0.f ? -1.f : 1.f;
		mobileBox.Translate(Vec3(direction * xDistance, 0.f, 0.f));
		return Vec3(0.f, 1.f, 1.f);
	}
	else if (yDistance <= xDistance && yDistance <= zDistance)
	{
		float direction = displacement.y > 0.f ? -1.f : 1.f;
		mobileBox.Translate(Vec3(0.f, direction * yDistance, 0.f));
		return Vec3(1.f, 0.f, 1.f);
	}
	else if (zDistance <= xDistance && zDistance <= yDistance)
	{
		float direction = displacement.z > 0.f ? -1.f : 1.f;
		mobileBox.Translate(Vec3(0.f, 0.f, direction * zDistance));
		return Vec3(1.f, 1.f, 0.f);
	}
	else
	{
		return Vec3::ONE;
	}
}


bool PushSphereOutOfPoint3D(Vec3& mobileSphereCenter, float sphereRadius, Vec3 const& fixedPoint)
{
	if (!IsPointInsideSphere3D(fixedPoint, mobileSphereCenter, sphereRadius)) return false;
	float distance = GetDistance3D(mobileSphereCenter, fixedPoint);
	float overlapDepth = sphereRadius - distance;
	Vec3 pushDisplacement = mobileSphereCenter - fixedPoint;
	pushDisplacement.SetLength(overlapDepth);
	mobileSphereCenter += pushDisplacement;
	return true;
}


bool PushSphereOutOfOBB3D(Vec3& mobileSphereCenter, float sphereRadius, OBB3 const& fixedBox)
{
	Vec3 nearestPoint = GetNearestPointOnOBB3D(mobileSphereCenter, fixedBox);
	if (!IsPointInsideSphere3D(nearestPoint, mobileSphereCenter, sphereRadius)) return false;
	PushSphereOutOfPoint3D(mobileSphereCenter, sphereRadius, nearestPoint);
	return true;
}


bool PushSphereOutCapsule3D(Vec3& mobileSphereCenter, float sphereRadius, Capsule3 const& capsule)
{
	Vec3 nearestPoint = GetNearestPointOnCapsule3D(mobileSphereCenter, capsule);
	if (!IsPointInsideSphere3D(nearestPoint, mobileSphereCenter, sphereRadius)) return false;
	PushSphereOutOfPoint3D(mobileSphereCenter, sphereRadius, nearestPoint);
	return true;
}


bool BounceDiscOffPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2& mobileVelocity, Vec2 const& fixedPoint, float elasticity)
{
	//if(!IsPointInsideDisc2D(fixedPoint, mobileDiscCenter, discRadius)) return false;
	//PushDiscOutOfPoint2D(mobileDiscCenter, discRadius, fixedPoint);
	UNUSED(discRadius)
	Vec2 normal = (fixedPoint - mobileDiscCenter).GetNormalized();
	Vec2 projectedLength = DotProduct2D(mobileVelocity, normal) * normal;
	mobileVelocity -= (projectedLength * (1.f + elasticity));
	return true;
}


bool BounceDiscOffDisc2D(Vec2& mobileDiscCenter, float mobileRadius, Vec2& mobileVeclocity, Vec2 const& fixedDiscCenter, float fixedRadius, float elasticity)
{
	if (!PushDiscOutOfDisc2D(mobileDiscCenter, mobileRadius, fixedDiscCenter, fixedRadius)) return false;
	Vec2 nearnestPointOnDisc = GetNearestPointOnDisc2D(mobileDiscCenter, fixedDiscCenter, fixedRadius);
	return BounceDiscOffPoint2D(mobileDiscCenter, mobileRadius, mobileVeclocity, nearnestPointOnDisc, elasticity);
}


bool BounceDiscOffAABB2D(Vec2& mobileDiscCenter, float discRadius, Vec2& mobileVelocity, AABB2 const& fixedBox, float elasticity)
{
	if (!PushDiscOutOfAABB2D(mobileDiscCenter, discRadius, fixedBox)) return false;
	Vec2 nearestPoint = GetNearestPointOnAABB2D(mobileDiscCenter, fixedBox);
	return BounceDiscOffPoint2D(mobileDiscCenter, discRadius, mobileVelocity, nearestPoint, elasticity);
}


bool BounceDiscOffOBB2D(Vec2& mobileDiscCenter, float discRadius, Vec2& mobileVelocity, OBB2 const& fixedBox, float elasticity)
{
	if (!PushDiscOutOfOBB2D(mobileDiscCenter, discRadius, fixedBox)) return false;
	Vec2 nearestPoint = GetNearestPointOnOBB2D(mobileDiscCenter, fixedBox);
	return BounceDiscOffPoint2D(mobileDiscCenter, discRadius, mobileVelocity, nearestPoint, elasticity);
}


bool BounceDiscOffCapsule2D(Vec2& mobileDiscCenter, float discRadius, Vec2& mobileVelocity, Capsule2 const& fixedCapsule, float elasticity)
{
	if (!PushDiscOutOfCapsule2D(mobileDiscCenter, discRadius, fixedCapsule)) return false;
	Vec2 nearestPoint = GetNearestPointOnCapsule2D(mobileDiscCenter, fixedCapsule);
	return BounceDiscOffPoint2D(mobileDiscCenter, discRadius, mobileVelocity, nearestPoint, elasticity);
}


bool BounceDiscsOffEachOther2D(Vec2& aCenter, float aRadius, Vec2& aVelocity, Vec2& bCenter, float bRadius, Vec2& bVelocity, float elasticity)
{
	float totalRadius = aRadius + bRadius;
	if (GetDistanceSquared2D(aCenter, bCenter) > (totalRadius * totalRadius)) return false;
	PushDiscsOutOfEachOther2D(aCenter, aRadius, bCenter, bRadius);
	Vec2 normal = (bCenter - aCenter).GetNormalized();

	if (DotProduct2D(normal, aVelocity) > DotProduct2D(normal, bVelocity))
	{
		Vec2 swapVelocityA = GetProjectedOnto2D(aVelocity, normal);
		Vec2 tangentialVelocityA = aVelocity - swapVelocityA;
		Vec2 swapVelocityB = GetProjectedOnto2D(bVelocity, -normal);
		Vec2 tangentialVelocityB = bVelocity - swapVelocityB;
		aVelocity = tangentialVelocityA + swapVelocityB * elasticity;
		bVelocity = tangentialVelocityB + swapVelocityA * elasticity;
	}
	return true;
}


bool BounceSphereOffPoint3D(Vec3& mobileSphereCenter, float sphereRadius, Vec3& mobileVelocity, Vec3 const& fixedPoint, float elasticity)
{
	if (!IsPointInsideSphere3D(fixedPoint, mobileSphereCenter, sphereRadius)) return false;
	PushSphereOutOfPoint3D(mobileSphereCenter, sphereRadius, fixedPoint);
	Vec3 normal = (fixedPoint - mobileSphereCenter).GetNormalized();
	Vec3 projectedLength = DotProduct3D(mobileVelocity, normal) * normal;
	mobileVelocity -= (projectedLength * (1.f + elasticity));
	return true;
}


bool BounceSphereOffOBB3D(Vec3& mobileSphereCenter, float sphereRadius, Vec3& mobileVelocity, OBB3 const& fixedBox, float elasticity)
{
	if (!PushSphereOutOfOBB3D(mobileSphereCenter, sphereRadius, fixedBox)) return false;
	Vec3 nearestPoint = GetNearestPointOnOBB3D(mobileSphereCenter, fixedBox);
	BounceSphereOffPoint3D(mobileSphereCenter, sphereRadius, mobileVelocity, nearestPoint, elasticity);
	return true;
}


bool BounceSphereOffCapsule3D(Vec3& mobileSphereCenter, float sphereRadius, Vec3& mobileVelocity, Capsule3 const& fixedCapsule, float elasticity)
{
	if (!PushSphereOutCapsule3D(mobileSphereCenter, sphereRadius, fixedCapsule)) return false;
	Vec3 nearestPoint = GetNearestPointOnCapsule3D(mobileSphereCenter, fixedCapsule);
	BounceSphereOffPoint3D(mobileSphereCenter, sphereRadius, mobileVelocity, nearestPoint, elasticity);
	return true;
}


bool BounceSpheresOffEachOther3D(Vec3& aCenter, float aRadius, Vec3& aVelocity, Vec3& bCenter, float bRadius, Vec3& bVelocity, float elasticity)
{
	Vec3 displacement = bCenter - aCenter;
	float distanceSquare = displacement.GetLengthSquared();
	float totalRadius = aRadius + bRadius;
	if (distanceSquare >= (totalRadius * totalRadius)) return false;

	displacement.SetLength(totalRadius - displacement.GetLength());
	aCenter -= 0.5f * displacement;
	bCenter += 0.5f * displacement;

	Vec3 normal = displacement.GetNormalized();

	if (DotProduct3D(normal, aVelocity) > DotProduct3D(normal, bVelocity))
	{
		Vec3 swapVelocityA = GetProjectedOnto3D(aVelocity, normal);
		Vec3 swapVelocityB = GetProjectedOnto3D(bVelocity, -normal);
		aVelocity += (-swapVelocityA + swapVelocityB * elasticity);
		bVelocity += (-swapVelocityB + swapVelocityA * elasticity);
	}
	return true;
}


void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation) {
	//scale
	float x = uniformScale * posToTransform.x;
	float y = uniformScale * posToTransform.y;

	//rotate
	float r = sqrtf((x * x) + (y * y));
	float thetaDegrees = Atan2Degrees(y, x);
	thetaDegrees += rotationDegrees;
	x = r * CosDegrees(thetaDegrees);
	y = r * SinDegrees(thetaDegrees);

	//translate
	x += translation.x;
	y += translation.y;

	posToTransform.x = x;
	posToTransform.y = y;
}


void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	posToTransform = translation + (iBasis * posToTransform.x) + (jBasis * posToTransform.y);
}


void TransformPositionXY3D(Vec3& posToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY)
{
	//scale
	float x = scaleXY * posToTransform.x;
	float y = scaleXY * posToTransform.y;

	//rotate
	float r = sqrtf((x * x) + (y * y));
	float thetaDegrees = Atan2Degrees(y, x);
	thetaDegrees += zRotationDegrees;
	x = r * CosDegrees(thetaDegrees);
	y = r * SinDegrees(thetaDegrees);

	//translate
	x += translationXY.x;
	y += translationXY.y;

	posToTransform.x = x;
	posToTransform.y = y;
}


void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY)
{
	Vec2 posToTransformXY = translationXY + (iBasis * posToTransform.x) + (jBasis * posToTransform.y);
	posToTransform.x = posToTransformXY.x;
	posToTransform.y = posToTransformXY.y;
}


void TransformPositionXYZ3D(Vec3& posToTransform, Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis, Vec3 const& translationXYZ)
{
	Vec3 posToTransformXYZ = translationXYZ + (iBasis * posToTransform.x) + (jBasis * posToTransform.y) + (kBasis * posToTransform.z);
	posToTransform.x = posToTransformXYZ.x;
	posToTransform.y = posToTransformXYZ.y;
	posToTransform.z = posToTransformXYZ.z;
}


float RangeMap(float input, float inMin, float inMax, float outMin, float outMax)
{
	float fraction = GetFractionWithin(input, inMin, inMax);
	float output = Interpolate(outMin, outMax, fraction);
	return output;
}


float GetFractionWithin(float input, float inMin, float inMax)
{
	if (inMin == inMax) return 0.5f;

	return (input - inMin) / (inMax - inMin);
}


float Interpolate(float outMin, float outMax,float fraction)
{
	return fraction * (outMax - outMin) + outMin;
}


Vec2 Interpolate(Vec2 outMin, Vec2 outMax, float fraction)
{
	float x = Interpolate(outMin.x, outMax.x, fraction);
	float y = Interpolate(outMin.y, outMax.y, fraction);
	return Vec2(x, y);
}


float RangeMapClamped(float input, float inMin, float inMax, float outMin, float outMax)
{
	float clampedInput = Clamp(input, inMin, inMax);
	return RangeMap(clampedInput, inMin, inMax, outMin, outMax);
}


int Clamp(int input, int outMin, int outMax)
{
	int output = input <= outMax ? input : outMax;
	output = output >= outMin ? output : outMin;
	return output;
}


float Clamp(float input, float outMin, float outMax)
{
	float output = input <= outMax ? input : outMax;
	output = output >= outMin ? output : outMin;
	return output;
}


float ClampZeroToOne(float input)
{
	return Clamp(input, 0.f, 1.f);
}


int RoundDownToInt(float input)
{
	return static_cast<int>(floorf(input));
}


float NormalizeByte(unsigned char byteValue)
{
	if (byteValue == 255) return 1.f;
	return static_cast<float>(byteValue) / 256.f;
}


unsigned char DenormalizeByte(float zeroToOne)
{
	if (zeroToOne == 1.f) return 255;
	return static_cast<unsigned char>(RoundDownToInt(zeroToOne * 256.f));
}


float ComputeCubicBezier1D(float a, float b, float c, float d, float t)
{
	float ab = Interpolate(a, b, t);
	float bc = Interpolate(b, c, t);
	float cd = Interpolate(c, d, t);

	float abc = Interpolate(ab, bc, t);
	float bcd = Interpolate(bc, cd, t);
	
	float abcd = Interpolate(abc, bcd, t);

	return abcd;
}


float ComputeQuniticBezier1D(float a, float b, float c, float d, float e, float f, float t)
{
	float ab = Interpolate(a, b, t);
	float bc = Interpolate(b, c, t);
	float cd = Interpolate(c, d, t);
	float de = Interpolate(d, e, t);
	float ef = Interpolate(e, f, t);

	float abc = Interpolate(ab, bc, t);
	float bcd = Interpolate(bc, cd, t);
	float cde = Interpolate(cd, de, t);
	float def = Interpolate(de, ef, t);

	float abcd = Interpolate(abc, bcd, t);
	float bcde = Interpolate(bcd, cde, t);
	float cdef = Interpolate(cde, def, t);

	float abcde = Interpolate(abcd, bcde, t);
	float bcdef = Interpolate(bcde, cdef, t);

	float abcdef = Interpolate(abcde, bcdef, t);

	return abcdef;
}


float Identity(float t)
{
	return t;
}


float SmoothStart2(float t)
{
	return t * t;
}


float SmoothStart3(float t)
{
	return t * t * t;
}


float SmoothStart4(float t)
{
	return t * t * t * t;
}


float SmoothStart5(float t)
{
	return t * t * t * t * t;
}


float SmoothStart6(float t)
{
	return t * t * t * t * t * t;
}


float SmoothStop2(float t)
{
	float nt = 1.f - t;
	return 1.f - (nt * nt);
}


float SmoothStop3(float t)
{
	float nt = 1.f - t;
	return 1.f - (nt * nt * nt);
}


float SmoothStop4(float t)
{
	float nt = 1.f - t;
	return 1.f - (nt * nt * nt * nt);
}


float SmoothStop5(float t)
{
	float nt = 1.f - t;
	return 1.f - (nt * nt * nt * nt * nt);
}


float SmoothStop6(float t)
{
	float nt = 1.f - t;
	return 1.f - (nt * nt * nt * nt * nt * nt);
}


float SmoothStep3(float t)
{
	// ComputeCubicBezier1D(0, 0, 1, 1, t)
	// Interpolate(SmoothStart2(t), SmoothStop2(t), t)
	return (3.f * (t * t)) - (2.f * (t * t * t));
}


float SmoothStep5(float t)
{
	// ComputeQuinticBezier1D(0, 0, 0, 1, 1, 1, t)
	return (10.f * (t * t * t)) - (15.f * (t * t * t * t)) + (6.f * (t * t * t * t * t));
}


float Hesitate3(float t)
{
	// ComputeCubicBezier1D(0, 1, 0, 1, t)
	return (4.f * (t * t * t)) - (6.f * (t * t)) + (3.f * t);
}


float Hesitate5(float t)
{
	// ComputeQuinticBezier1D(0, 1, 0, 1, 0, 1, t)
	return (16.f * (t * t * t * t * t)) - (40.f * (t * t * t * t)) + (40.f * (t * t * t)) - (20.f * (t * t)) + (5.f * t);
}


