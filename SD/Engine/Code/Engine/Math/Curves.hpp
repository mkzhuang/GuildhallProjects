#pragma once
#include "Engine/Math/Vec2.hpp"

#include <vector>

class BezierCurve2D;
class HermiteCurve2D;

class Spline
{
public:
	Spline();
	Spline(std::vector<Vec2> points);
	~Spline() {}

	Vec2 EvaluateAtParametric(float parametric) const;
	float GetApproximateLength(int numSubdivisions = 64) const;
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions = 64) const;

public:
	std::vector<Vec2> m_points;
	std::vector<Vec2> m_velocities;
	std::vector<HermiteCurve2D> m_hermiteCurves;
};


class BezierCurve2D
{
public:
	BezierCurve2D() {}
	BezierCurve2D(std::vector<Vec2> const& points);
	explicit BezierCurve2D(HermiteCurve2D const& hermiteCurve);
	~BezierCurve2D() {}

	Vec2 EvaluateAtParametric(float parametricZeroToOne) const;
	float GetApproximateLength(int numSubdivisions = 64) const;
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions = 64) const;

private:
	std::vector<Vec2> GetPointsOnLine(int numSubdivisions = 64) const;

public:
	std::vector<Vec2> m_points;
};


class HermiteCurve2D
{
public:
	HermiteCurve2D() {}
	HermiteCurve2D(std::vector<Vec2> const& points, std::vector<Vec2> const& velocities);
	explicit HermiteCurve2D(BezierCurve2D const& bezierCurve);
	~HermiteCurve2D() {}

	Vec2 EvaluateAtParametric(float parametricZeroToOne) const;
	float GetApproximateLength(int numSubdivisions = 64) const;
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions = 64) const;

public:
	std::vector<Vec2> m_points;
	std::vector<Vec2> m_velocities;
	BezierCurve2D m_bezierCurve;
};


