#pragma once
#include "Engine/Math/Vec3.hpp"

struct LineSegment3
{
public:
	Vec3 m_start;
	Vec3 m_end;

public:
	LineSegment3() {}
	~LineSegment3() {}
	explicit LineSegment3(Vec3 start, Vec3 end);

	Vec3 GetCenter() const;

	void Translate(Vec3 const& translation);
	void SetCenter(Vec3 const& newCenter);
};