#pragma once
#include "Engine/Math/Vec2.hpp"

struct LineSegment2
{
public:
	Vec2 m_start;
	Vec2 m_end;

public:
	LineSegment2() {}
	~LineSegment2() {}
	LineSegment2(LineSegment2 const& copy);
	explicit LineSegment2(Vec2 start, Vec2 end);
	explicit LineSegment2(float startX, float startY, float endX, float endY);

	Vec2 GetCenter() const;

	void Translate(Vec2 const& translation);
	void SetCenter(Vec2 const& newCenter);
	void RotateAboutCenter(float const& rotationDeltaDegrees);
};


