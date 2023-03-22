#include "Engine/Math/Curves.hpp"
#include "Engine/Math/MathUtils.hpp"

static size_t one = 1;

//Spline
Spline::Spline()
{
}


Spline::Spline(std::vector<Vec2> points)
	: m_points(points)
{
	m_velocities.push_back(Vec2::ZERO);
	for (int pointIndex = 1; pointIndex < (int)m_points.size() - 1; pointIndex++)
	{
		m_velocities.push_back(0.5f * (m_points[pointIndex + one] - m_points[pointIndex - one]));
	}
	m_velocities.push_back(Vec2::ZERO);

	int numOfCurves = (int)m_points.size() - 1;
	for (int curveIndex = 0; curveIndex < numOfCurves; curveIndex++)
	{
		std::vector<Vec2> hermitePoints;
		hermitePoints.push_back(m_points[curveIndex]);
		hermitePoints.push_back(m_points[curveIndex + one]);
		std::vector<Vec2> hermiteVelocities;
		hermiteVelocities.push_back(m_velocities[curveIndex]);
		hermiteVelocities.push_back(m_velocities[curveIndex + one]);
		HermiteCurve2D curve(hermitePoints, hermiteVelocities);
		m_hermiteCurves.push_back(curve);
	}
}


Vec2 Spline::EvaluateAtParametric(float parametric) const
{
	int section = RoundDownToInt(parametric);
	float parametricZeroToOne = parametric - static_cast<float>(section);

	return m_hermiteCurves[section].EvaluateAtParametric(parametricZeroToOne);
}


float Spline::GetApproximateLength(int numSubdivisions) const
{
	float totalLength = 0.f;
	for (int curveIndex = 0; curveIndex < (int)m_hermiteCurves.size(); curveIndex++)
	{
		totalLength += m_hermiteCurves[curveIndex].GetApproximateLength(numSubdivisions);
	}

	return totalLength;
}


Vec2 Spline::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions) const
{
	float length = 0.f;
	for (int curveIndex = 0; curveIndex < (int)m_hermiteCurves.size(); curveIndex++)
	{
		float subLength = m_hermiteCurves[curveIndex].GetApproximateLength(numSubdivisions);
		length += subLength;
		if (length >= distanceAlongCurve)
		{
			length -= subLength;
			float fractionLength = distanceAlongCurve - length;
			return m_hermiteCurves[curveIndex].EvaluateAtApproximateDistance(fractionLength, numSubdivisions);
		}
	}

	return Vec2::ZERO;
}


// Bezier Curve
BezierCurve2D::BezierCurve2D(std::vector<Vec2> const& points)
	: m_points(points)
{
}


BezierCurve2D::BezierCurve2D(HermiteCurve2D const& hermiteCurve)
{
	float oneThird = 1.f / 3.f;
	Vec2 pointB = hermiteCurve.m_points[0] + (oneThird * hermiteCurve.m_velocities[0]);
	Vec2 pointC = hermiteCurve.m_points[1] - (oneThird * hermiteCurve.m_velocities[1]);
	m_points.push_back(hermiteCurve.m_points[0]);
	m_points.push_back(pointB);
	m_points.push_back(pointC);
	m_points.push_back(hermiteCurve.m_points[1]);
}


Vec2 BezierCurve2D::EvaluateAtParametric(float parametricZeroToOne) const
{
	std::vector<Vec2> copy = m_points;
	int iterations = static_cast<int>(copy.size()) - 1;
	while (iterations > 0)
	{
		for (int pointIndex = 0; pointIndex < iterations; pointIndex++)
		{
			copy[pointIndex] = Interpolate(copy[pointIndex], copy[pointIndex + one], parametricZeroToOne);
		}
		iterations--;
	}
	
	return copy[0];
}


float BezierCurve2D::GetApproximateLength(int numSubdivisions) const
{
	std::vector<Vec2> const& linePoints = GetPointsOnLine(numSubdivisions);
	float length = 0.f;
	for (int currentPoint = 0; currentPoint < (int)linePoints.size() - 1; currentPoint++)
	{
		float subLength = (linePoints[currentPoint + one] - linePoints[currentPoint]).GetLength();
		length += subLength;
	}

	return length;
}


Vec2 BezierCurve2D::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions) const
{
	std::vector<Vec2> const& linePoints = GetPointsOnLine(numSubdivisions);
	float length = 0.f;
	for (int currentPoint = 0; currentPoint < (int)linePoints.size() - 1; currentPoint++)
	{
		float subLength = (linePoints[currentPoint + one] - linePoints[currentPoint]).GetLength();
		length += subLength;
		if (length >= distanceAlongCurve)
		{
			length -= subLength;
			float fractionLength = distanceAlongCurve - length;
			float ratioOnCurrentLine = fractionLength / (linePoints[currentPoint + one] - linePoints[currentPoint]).GetLength();
			return Interpolate(linePoints[currentPoint], linePoints[currentPoint + one], ratioOnCurrentLine);
		}
	}

	return Vec2::ZERO;
}


std::vector<Vec2> BezierCurve2D::GetPointsOnLine(int numSubdivisions) const
{
	float fractionPerDivision = 1.f / numSubdivisions;
	std::vector<Vec2> linePoints;
	for (int currentDivision = 0; currentDivision <= numSubdivisions; currentDivision++)
	{
		Vec2 point = EvaluateAtParametric(static_cast<float>(currentDivision) * fractionPerDivision);
		linePoints.push_back(point);
	}

	return linePoints;
}



// Hermite Curve
HermiteCurve2D::HermiteCurve2D(std::vector<Vec2> const& points, std::vector<Vec2> const& velocities)
	: m_points(points)
	, m_velocities(velocities)
{
	m_bezierCurve = BezierCurve2D(*this);
}


HermiteCurve2D::HermiteCurve2D(BezierCurve2D const& bezierCurve)
{
	m_points.push_back(bezierCurve.m_points[0]);
	m_points.push_back(bezierCurve.m_points[3]);
	Vec2 velocityU = 3.f * (bezierCurve.m_points[1] - bezierCurve.m_points[0]);
	Vec2 velocityV = 3.f * (bezierCurve.m_points[3] - bezierCurve.m_points[2]);
	m_velocities.push_back(velocityU);
	m_velocities.push_back(velocityV);
	m_bezierCurve = bezierCurve;
}


Vec2 HermiteCurve2D::EvaluateAtParametric(float parametricZeroToOne) const
{
	return m_bezierCurve.EvaluateAtParametric(parametricZeroToOne);
}


float HermiteCurve2D::GetApproximateLength(int numSubdivisions) const
{
	return m_bezierCurve.GetApproximateLength(numSubdivisions);
}


Vec2 HermiteCurve2D::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions) const
{
	return m_bezierCurve.EvaluateAtApproximateDistance(distanceAlongCurve, numSubdivisions);
}


