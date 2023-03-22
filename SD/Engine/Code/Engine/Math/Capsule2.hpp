#pragma once
#include "Engine/Math/LineSegment2.hpp"

struct Capsule2
{
public:
	LineSegment2 m_bone;
	float m_radius = 0.f;

public:
	Capsule2() {}
	~Capsule2() {}
	Capsule2(Capsule2 const& copy);
	explicit Capsule2(LineSegment2 bone, float radius);

	void Translate(Vec2 const& translation);
	void SetCenter(Vec2 const& newCenter);
	void RotateAboutCenter(float const& rotationDeltaDegrees);
};


