#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"

struct EulerAngles
{
public:
	EulerAngles() = default;
	EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees);

	static const EulerAngles ZERO;
	
	void GetAsVectors_XFwd_YLeft_ZUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis) const;
	Mat44 GetAsMatrix_XFwd_YLeft_ZUp() const;
	void SetFromText(std::string const& text);
	
	EulerAngles const operator+(const EulerAngles& toAdd) const;
	EulerAngles const operator*(const float multiplier) const;
	
	void operator+=(const EulerAngles& toAdd);
	void operator*=(const float multiplier);

public:
	float m_yawDegrees		= 0.f;
	float m_pitchDegrees	= 0.f;
	float m_rollDegrees		= 0.f;
};


