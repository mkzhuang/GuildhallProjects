#pragma once
#include "Engine//Math/Vec2.hpp"

struct Plane2D
{
public:
	Plane2D() {}
	Plane2D(Vec2& planeNormal, float distanceFromOrigin);
	~Plane2D() {}

	bool operator==(Plane2D const& compare) const;

public:
	Vec2 m_planeNormal = Vec2::ZERO;
	float m_distanceFromOrigin = 1.f;
};