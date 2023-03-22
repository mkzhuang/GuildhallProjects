#include "Engine/Math/ConvexPoly2D.hpp"
#include "Engine/Math/MathUtils.hpp"

ConvexPoly2D::ConvexPoly2D(std::vector<Vec2> const& orderedPoints)
	: m_orderedPoints(orderedPoints)
{
}


std::vector<Vec2> const& ConvexPoly2D::GetPoints() const
{
	return m_orderedPoints;
}


void ConvexPoly2D::SetPoints(std::vector<Vec2> const& orderedPoints)
{
	m_orderedPoints.clear();
	m_orderedPoints = orderedPoints;
}


void ConvexPoly2D::Translate(Vec2 const& translation)
{
	for (Vec2& point : m_orderedPoints)
	{
		point += translation;
	}
}


void ConvexPoly2D::RotateAroundPoint(Vec2 const& point, float rotateDegrees)
{
	for (Vec2& convexPoint : m_orderedPoints)
	{
		Vec2 displacement = convexPoint - point;
		displacement.RotateDegrees(rotateDegrees);
		convexPoint = point + displacement;
	}
}


void ConvexPoly2D::ScaleAroundPoint(Vec2 const& point, float scale)
{
	for (Vec2& convexPoint : m_orderedPoints)
	{
		Vec2 displacement = convexPoint - point;
		displacement *= scale;
		convexPoint = point + displacement;
	}
}


 void ConvexPoly2D::ToBoundingPlanes(std::vector<Plane2D>& boundingPlanes)
{
	for (int index = 0; index < static_cast<int>(m_orderedPoints.size() - 1); index++)
	{
		Vec2 displacement = m_orderedPoints[index + 1] - m_orderedPoints[index];
		displacement.Normalize();
		displacement.RotateMinus90Degrees();
		float length = DotProduct2D(m_orderedPoints[index], displacement);
		Plane2D plane(displacement, length);
		boundingPlanes.push_back(plane);
	}

	Vec2 displacement =  m_orderedPoints[0] - m_orderedPoints[m_orderedPoints.size() - 1];
	displacement.Normalize();
	displacement.RotateMinus90Degrees();
	float length = DotProduct2D(m_orderedPoints[0], displacement);
	Plane2D plane(displacement, length);
	boundingPlanes.push_back(plane);
}


