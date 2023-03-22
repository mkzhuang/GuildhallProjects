#pragma once
#include "Engine/Math/Vec3.hpp"

struct AABB3
{
public:
	~AABB3() {}
	AABB3() {}
	AABB3(AABB3 const& copyFrom);
	explicit AABB3(Vec3 mins, Vec3 maxs);
	explicit AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

	static AABB3 const ZERO_TO_ONE;

	void GetCorners(Vec3* corners) const;
	bool IsPointInside(Vec3 const& point) const;
	Vec3 GetCenter() const;
	Vec3 GetCenterAtMinZ() const;
	Vec3 GetHalfDimensions() const;
	//Vec2 GetNearestPoint(Vec3 const& referencePoint) const;
	//Vec2 GetPointAtUV(Vec3 const& uv) const;
	//Vec2 GetUVForPoint(Vec3 const& point) const;

	void Translate(Vec3 const& translation);
	void SetCenter(Vec3 const& newCenter);
	void SetCenterAtMinZ(Vec3 const& newCenter);
	//void SetDimensions(Vec3 const& dimensions);
	//void StretchToIncludePoint(Vec3 const& point);
	//void AlignBoxWithin(AABB3& box, Vec3 const& alignment) const;

public:
	Vec3 m_mins;
	Vec3 m_maxs;
};