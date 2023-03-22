#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"

struct OBB3
{
public:
	Vec3 m_center;
	EulerAngles m_orientation;
	Vec3 m_halfDimensions;

public:
	OBB3() {}
	~OBB3() {}
	explicit OBB3(Vec3 center, EulerAngles orientation, Vec3 halfDimensions);

	void GetCornerPoints(Vec3* out_cornerWorldPositions) const;
	Vec3 GetLocalPosForWorldPos(Vec3 worldPos) const;
	Vec3 GetWorldPosForLocalPos(Vec3 localPos) const;
};