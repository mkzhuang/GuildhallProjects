#include "Engine/Math/Plane2D.hpp"

Plane2D::Plane2D(Vec2& planeNormal, float distanceFromOrigin)
	: m_planeNormal(planeNormal)
	, m_distanceFromOrigin(distanceFromOrigin)
{
}


bool Plane2D::operator==(Plane2D const& compare) const
{
	return m_planeNormal == compare.m_planeNormal && m_distanceFromOrigin == compare.m_distanceFromOrigin;
}

