#pragma once
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"

#include <vector>

class ConvexHull2D
{
public:
	ConvexHull2D() {}
	ConvexHull2D(std::vector<Plane2D> const& boundingPlanes);
	ConvexHull2D(ConvexPoly2D& convexPoly2D);
	~ConvexHull2D() {}
	std::vector<Plane2D> const& GetBoundingPlanes() const;
	void SetBoundingPlanes(std::vector<Plane2D>& boundingPlanes);

private:
	std::vector<Plane2D> m_boundingPlanes;
};