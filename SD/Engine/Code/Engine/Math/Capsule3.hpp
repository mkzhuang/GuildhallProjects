#pragma once
#include "Engine/Math/LineSegment3.hpp"

struct Capsule3
{
public:
	LineSegment3 m_bone;
	float m_radius = 0;

public:
	Capsule3() {}
	~Capsule3() {}
	explicit Capsule3(LineSegment3 bone, float radius);

	void Translate(Vec3 const& translation);
	void SetCenter(Vec3 const& newCenter);
};


