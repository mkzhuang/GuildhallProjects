#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

typedef float (*EasingFunction)(float t);

struct Vec4;
struct IntVec2;
struct LineSegment2;
struct LineSegment3;
struct AABB2;
struct AABB3;
struct OBB2;
struct OBB3;
struct Capsule2;
struct Capsule3;

struct RaycastResult2D
{
public:
	RaycastResult2D() {}
	RaycastResult2D(bool didImpact, Vec2 impactPosition, float impactDistance, Vec2 impactSurfaceNormal,
					Vec2 startPosition, Vec2 forwardNormal, float maxDistance);

public:
	bool m_didImpact = false;
	Vec2 m_impactPosition;
	float m_impactDistance = 0.f;
	Vec2 m_impactSurfaceNormal;

	Vec2 m_startPosition;
	Vec2 m_forwardNormal;
	float m_maxDistance = 1.f;
};

struct RaycastResult3D
{
public:
	RaycastResult3D() {}
	RaycastResult3D(bool didImpact, Vec3 impactPosition, float impactDistance, Vec3 impactSurtaceNormal,
				    Vec3 startPosition, Vec3 forwardNormal, float maxDistance);

public :
	bool m_didImpact = false;
	Vec3 m_impactPosition;
	float m_impactDistance = 0.f;
	Vec3 m_impactSurfaceNormal;

	Vec3 m_startPosition;
	Vec3 m_forwardNormal;
	float m_maxDistance = 1.f;
};

//Angle utilities
float ConvertDegreesToRadians(float degrees);
float ConvertRadiansToDegrees(float radians);
float CosDegrees(float degrees);
float SinDegrees(float degrees);
float Atan2Degrees(float y, float x);
float GetShortestAngularDispDegrees(float fromDegrees, float toDegrees);
float GetTurnedTowardDegrees(float fromDegrees, float toDegees, float maxDeltaDegrees);

//Quaternion Tool, not 100% sure if they are correct
Vec4 GetQuaternionsForAngleAxis(Vec3 const& axis);
Vec4 MultiplyQuaternions(Vec4 const& q1, Vec4 const& q2);
Vec3 GetAngleAxisForQuaternions(Vec4 const& quaternions);

//Basic 2D & 3D utilities
float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB);
float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB);
float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB);
float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB);
int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB);
float DotProduct2D(Vec2 const& a, Vec2 const& b);
float DotProduct3D(Vec3 const& a, Vec3 const& b);
float DotProduct4D(Vec4 const& a, Vec4 const& b);
float CrossProduct2D(Vec2 const& a, Vec2 const& b);
Vec3 const CrossProduct3D(Vec3 const& a, Vec3 const& b);
float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto);
Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto);
float GetProjectedLength3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto);
Vec3 const GetProjectedOnto3D(Vec3 const& vectorToProject, Vec3 const& vectorProjectOnto);
float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b);
RaycastResult2D const RaycastVsDisc2D(Vec2 const& startPos, Vec2 const& forwardNormal, float maxDistance, Vec2 const& discCenter, float discRadius);
RaycastResult2D const RaycastVsAABB2D(Vec2 const& startPos, Vec2 const& forwardNormal, float maxDistance, AABB2 const& box);
RaycastResult2D const RaycastVsOBB2D(Vec2 const& startPos, Vec2 const& forwardNormal, float maxDistance, OBB2 const& box);
RaycastResult2D const RaycastVsLineSegment2D(Vec2 const& startPos, Vec2 const& forwardNormal, float maxDistance, LineSegment2 const& line);
RaycastResult3D const RaycastVsSphere3D(Vec3 const& startPos, Vec3 const& forwardNormal, float maxDistance, Vec3 const& sphereCenter, float sphereRadius);
RaycastResult3D const RaycastVsAABB3D(Vec3 const& startPos, Vec3 const& forwardNormal, float maxDistance, AABB3 const& box);
RaycastResult3D const RaycastVsZCylinder3D(Vec3 const& startPos, Vec3 const& forwardNormal, float maxDistance, Vec3 const& cylinderCenterBottom, float cylinderRadius, float cylinderHeight);

//Geometric query utilities
bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius);
bool IsPointInsideSphere3D(Vec3 const& point, Vec3 const& sphereCenter, float sphereRadius);
bool IsPointInsideZCylinder3D(Vec3 const& point, Vec3 const& cylinderBottomCenter, float cylinderRadius, float cylinderHeight);
bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box);
bool IsPointInsideAABB3D(Vec3 const& point, AABB3 const& box);
bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule);
bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientedBox);
bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius);
bool DoDiscsOverlap2D(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB);
bool DoAABB2sOverlap2D(AABB2 const& boxA, AABB2 const& boxB);
bool DoesAABB2OverlapWithDisc2D(AABB2 const& box, Vec2 const& discCenter, float discRadius);
bool DoesOBB2OverlapWithDisc2D(OBB2 const& box, Vec2 const& discCenter, float discRadius);
bool DoSpheresOverlap3D(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB);
bool DoAABB3sOverlap3D(AABB3 const& boxA, AABB3 const& boxB);
bool DoZCylindersOverlap3D(Vec3 const& bottomCenterA, float radiusA, float heightA, Vec3 const& bottomCenterB, float radiusB, float heightB);
bool DoesSphereOverlapWithAABB3D(Vec3 const& sphereCenter, float sphereRadius, AABB3 const& box);
bool DoesSphereOverlapWithOBB3(Vec3 const& sphereCenter, float sphereRadius, OBB3 const& box);
bool DoesSphereOverlapWithZCylinder3D(Vec3 const& sphereCenter, float sphereRadius, Vec3 const& cylinderBottomCenter, float cylinderRadius, float cylinderHeight);
bool DoesAABB3OverlapWithZCylinder3D(AABB3 const& box, Vec3 const& cylinderBottomCenter, float cylinderRadius, float cylinderHeight);
Vec2 const GetNearestPointOnDisc2D(Vec2 const& referencePos, Vec2 const& discCenter, float discRadius);
Vec2 const GetNearestPointOnAABB2D(Vec2 const& referencePos, AABB2 const& box);
Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePos, LineSegment2 const& infiniteLine);
Vec2 const GetNearestPointOnLineSegement2D(Vec2 const& referencePos, LineSegment2 const& lineSegement);
Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePos, Capsule2 const& capsule);
Vec2 const GetNearestPointOnOBB2D(Vec2 const& referencePos, OBB2 const& orientedBox);
Vec3 const GetNearestPointOnSphere3D(Vec3 const& referencePos, Vec3 const& sphereCenter, float sphereRadius);
Vec3 const GetNearestPointOnAABB3D(Vec3 const& referencePos, AABB3 const& box);
Vec3 const GetNearestPointOnOBB3D(Vec3 const& referencePos, OBB3 const& box);
Vec3 const GetNearestPointOnLineSegment3D(Vec3 const& referencePos, LineSegment3 const& lineSegment);
Vec3 const GetNearestPointOnCapsule3D(Vec3 const& referencePos, Capsule3 const& capsule);
Vec3 const GetNearestPointOnZCylinder3D(Vec3 const& referencePos, Vec3 const& cylinderBottomCenter, float cylinderRadius, float cylinderHeight);
bool PushDiscOutOfPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2 const& fixedPoint);
bool PushDiscOutOfDisc2D(Vec2& mobileDiscCenter, float mobileRadius, Vec2 const& fixedDiscCenter, float fixedRadius);
bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRaiuds, Vec2& bCenter, float bRadius);
bool PushDiscOutOfAABB2D(Vec2& mobileDiscCenter, float discRadius, AABB2 const& fixedBox);
bool PushDiscOutOfOBB2D(Vec2& mobileDiscCenter, float discRadius, OBB2 const& fixedBox);
bool PushDiscOutOfCapsule2D(Vec2& mobileDiscCenter, float discRadius, Capsule2 const& capsule);
Vec3 const PushAABB3OutOfAABB3D(AABB3& mobileBox, AABB3 const& fixedBox);
bool PushSphereOutOfPoint3D(Vec3& mobileSphereCenter, float sphereRadius, Vec3 const& fixedPoint);
bool PushSphereOutOfOBB3D(Vec3& mobileSphereCenter, float sphereRadius, OBB3 const& fixedBox);
bool PushSphereOutCapsule3D(Vec3& mobileSphereCenter, float sphereRadius, Capsule3 const& capsule);
bool BounceDiscOffPoint2D(Vec2& mobileDiscCenter, float discRadius, Vec2& mobileVelocity, Vec2 const& fixedPoint, float elasticity);
bool BounceDiscOffDisc2D(Vec2& mobileDiscCenter, float mobileRadius, Vec2& mobileVelocity, Vec2 const& fixedDiscCenter, float fixedRadius, float elasticity);
bool BounceDiscOffAABB2D(Vec2& mobileDiscCenter, float discRadius, Vec2& mobileVelocity, AABB2 const& fixedBox, float elasticity);
bool BounceDiscOffOBB2D(Vec2& mobileDiscCenter, float discRadius, Vec2& mobileVelocity, OBB2 const& fixedBox, float elasticity);
bool BounceDiscOffCapsule2D(Vec2& mobileDiscCenter, float discRadius, Vec2& mobileVelocity, Capsule2 const& fixedCapsule, float elasticity);
bool BounceDiscsOffEachOther2D(Vec2& aCenter, float aRadius, Vec2& aVelocity, Vec2& bCenter, float bRadius, Vec2& bVelocity, float elasticity);
bool BounceSphereOffPoint3D(Vec3& mobileSphereCenter, float sphereRadius, Vec3& mobileVelocity, Vec3 const& fixedPoint, float elasticity);
bool BounceSphereOffOBB3D(Vec3& mobileSphereCenter, float sphereRadius, Vec3& mobileVelocity, OBB3 const& fixedBox, float elasticity);
bool BounceSphereOffCapsule3D(Vec3& mobileSphereCenter, float sphereRadius, Vec3& mobileVelocity, Capsule3 const& fixedCapsule, float elasticity);
bool BounceSpheresOffEachOther3D(Vec3& aCenter, float aRadius, Vec3& aVelocity, Vec3& bCenter, float bRadius, Vec3& bVelocity, float elasticity);

//Transform utilities
void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation);
void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation);
void TransformPositionXY3D(Vec3& posToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY);
void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY);
void TransformPositionXYZ3D(Vec3& posToTransform, Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis, Vec3 const& translationXYZ);

//Lerping & Clamping
float RangeMap(float input, float inMin, float inMax, float outMin, float outMax);
float GetFractionWithin(float input, float in_Min, float inMax);
float Interpolate(float outMin, float outMax, float fraction);
Vec2 Interpolate(Vec2 outMin, Vec2 outMax, float fraction);
float RangeMapClamped(float input, float inMin, float inMax, float outMin, float outMax);
int Clamp(int input, int outMin, int outMax);
float Clamp(float input, float outMin, float outMax);
float ClampZeroToOne(float input);
int RoundDownToInt(float input);
float NormalizeByte(unsigned char byteValue);
unsigned char DenormalizeByte(float zeroToOne);

//easing, curves, splines
float ComputeCubicBezier1D(float a, float b, float c, float d, float t);
float ComputeQuniticBezier1D(float a, float b, float c, float d, float e, float f, float t);
float Identity(float t);
float SmoothStart2(float t);
float SmoothStart3(float t);
float SmoothStart4(float t);
float SmoothStart5(float t);
float SmoothStart6(float t);
float SmoothStop2(float t);
float SmoothStop3(float t);
float SmoothStop4(float t);
float SmoothStop5(float t);
float SmoothStop6(float t);
float SmoothStep3(float t);
float SmoothStep5(float t);
float Hesitate3(float t);
float Hesitate5(float t);


