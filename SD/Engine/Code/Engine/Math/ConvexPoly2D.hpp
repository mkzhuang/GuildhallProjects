#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Plane2D.hpp"

#include <vector>

class ConvexPoly2D
{
public:
	ConvexPoly2D() {}
	ConvexPoly2D(std::vector<Vec2> const& orderedPoints);
	~ConvexPoly2D() {}
	std::vector<Vec2> const& GetPoints() const;
	void SetPoints(std::vector<Vec2> const& orderedPoints);
	void Translate(Vec2 const& translation);
	void RotateAroundPoint(Vec2 const& point, float rotateDegrees);
	void ScaleAroundPoint(Vec2 const& point, float scale);
	void ToBoundingPlanes(std::vector<Plane2D>& boundingPlanes);
	
private:
	std::vector<Vec2> m_orderedPoints; // counterclockwise order
};