#include "Engine/Math/ConvexHull2D.hpp"

ConvexHull2D::ConvexHull2D(std::vector<Plane2D> const& boundingPlanes)
	: m_boundingPlanes(boundingPlanes)
{
}


ConvexHull2D::ConvexHull2D(ConvexPoly2D& convexPoly2D)
{
	convexPoly2D.ToBoundingPlanes(m_boundingPlanes);
}


std::vector<Plane2D> const& ConvexHull2D::GetBoundingPlanes() const
{
	return m_boundingPlanes;
}


void ConvexHull2D::SetBoundingPlanes(std::vector<Plane2D>& boundingPlanes)
{
	m_boundingPlanes.clear();
	m_boundingPlanes = boundingPlanes;
}


