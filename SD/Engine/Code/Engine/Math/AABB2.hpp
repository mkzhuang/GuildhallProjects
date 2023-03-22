#pragma once
#include "Engine/Math/Vec2.hpp"

struct AABB2
{
public:
	static AABB2 const ZERO_TO_ONE;
	static AABB2 const ONE_TO_ZERO;

	~AABB2() {}
	AABB2() {}
	AABB2(AABB2 const& copyFrom);
	explicit AABB2(Vec2 mins, Vec2 maxs);
	explicit AABB2(float minX, float minY, float maxX, float maxY);



	bool IsPointInside(Vec2 const& point) const;
	Vec2 GetCenter() const;
	Vec2 GetDimensions() const;
	Vec2 GetNearestPoint(Vec2 const& referencePoint) const;
	Vec2 GetPointAtUV(Vec2 const& uv) const;
	Vec2 GetUVForPoint(Vec2 const& point) const;
	AABB2 GetBoxWithIn(AABB2 uvs) const;

	void Translate(Vec2 const& translation);
	void SetCenter(Vec2 const& newCenter);
	void SetDimensions(Vec2 const& dimensions);
	void StretchToIncludePoint(Vec2 const& point);
	void AlignBoxWithin(AABB2& box, Vec2 const& alignment) const;

	void SetFromText(std::string const& text);

public:
	Vec2 m_mins;
	Vec2 m_maxs;
};


