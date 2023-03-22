#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

EulerAngles::EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees)
{
	m_yawDegrees = yawDegrees;
	m_pitchDegrees = pitchDegrees;
	m_rollDegrees = rollDegrees;
}


EulerAngles const EulerAngles::ZERO(0.f, 0.f, 0.f);


void EulerAngles::GetAsVectors_XFwd_YLeft_ZUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis) const
{
	float cy = CosDegrees(m_yawDegrees);
	float sy = SinDegrees(m_yawDegrees);
	float cp = CosDegrees(m_pitchDegrees);
	float sp = SinDegrees(m_pitchDegrees);
	float cr = CosDegrees(m_rollDegrees);
	float sr = SinDegrees(m_rollDegrees);

	out_forwardIBasis.x		= cy * cp;
	out_forwardIBasis.y		= sy * cp;
	out_forwardIBasis.z		= -sp;
	out_leftJBasis.x		= (cy * sp * sr) - (sy * cr);
	out_leftJBasis.y		= (sy * sp * sr) + (cy * cr);
	out_leftJBasis.z		= cp * sr;
	out_upKBasis.x			= (cy * sp * cr) + (sy * sr);
	out_upKBasis.y			= (sy * sp * cr) - (cy * sr);
	out_upKBasis.z			= cp * cr;
}


Mat44 EulerAngles::GetAsMatrix_XFwd_YLeft_ZUp() const
{
	float eulerMatrix[16] = {};
	float cy = CosDegrees(m_yawDegrees);
	float sy = SinDegrees(m_yawDegrees);
	float cp = CosDegrees(m_pitchDegrees);
	float sp = SinDegrees(m_pitchDegrees);
	float cr = CosDegrees(m_rollDegrees);
	float sr = SinDegrees(m_rollDegrees);

	eulerMatrix[0] = cy * cp;
	eulerMatrix[1] = sy * cp;
	eulerMatrix[2] = -sp;

	eulerMatrix[4] = (cy * sp * sr) - (sy * cr);
	eulerMatrix[5] = (sy * sp * sr) + (cy * cr);
	eulerMatrix[6] = cp * sr;

	eulerMatrix[8] = (cy * sp * cr) + (sy * sr);
	eulerMatrix[9] = (sy * sp * cr) - (cy * sr);
	eulerMatrix[10] = cp * cr;

	eulerMatrix[15] = 1.f;

	return Mat44(eulerMatrix);
}


void EulerAngles::SetFromText(std::string const& text)
{
	Strings tokens = SplitStringOnDelimiter(text, ',');

	if (tokens.size() == 3)
	{
		m_yawDegrees = static_cast<float>(atof(tokens[0].c_str()));
		m_pitchDegrees = static_cast<float>(atof(tokens[1].c_str()));
		m_rollDegrees = static_cast<float>(atof(tokens[2].c_str()));
	}
	else
	{
		ERROR_AND_DIE("Euler angles set from text input incorrect.");
	}
}


EulerAngles const EulerAngles::operator+(const EulerAngles& toAdd) const
{
	return EulerAngles(m_yawDegrees + toAdd.m_yawDegrees, m_pitchDegrees + toAdd.m_pitchDegrees, m_rollDegrees + m_rollDegrees);
}


EulerAngles const EulerAngles::operator*(const float multiplier) const
{
	return EulerAngles(m_yawDegrees * multiplier, m_pitchDegrees * multiplier, m_rollDegrees * multiplier);
}


void EulerAngles::operator+=(const EulerAngles& toAdd)
{
	m_yawDegrees += toAdd.m_yawDegrees;
	m_pitchDegrees += toAdd.m_pitchDegrees;
	m_rollDegrees += toAdd.m_rollDegrees;
}


void EulerAngles::operator*=(const float multiplier)
{
	m_yawDegrees *= multiplier;
	m_pitchDegrees *= multiplier;
	m_rollDegrees *= multiplier;
}


