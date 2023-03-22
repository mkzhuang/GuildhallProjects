#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

Vec2 AnalogJoystick::GetPosition() const
{
	return m_correctedPosition;
}


float AnalogJoystick::GetMagnitude() const
{
	return m_correctedPosition.GetLength();
}


float AnalogJoystick::GetOrientationDegrees() const
{
	return m_correctedPosition.GetOrientationDegrees();
}


Vec2 AnalogJoystick::GetRawUncorrectedPosition() const
{
	return m_rawPosition;
}


float AnalogJoystick::GetInnerDeadZoneFraction() const
{
	return m_InnerDeadzoneFraction;
}


float AnalogJoystick::GetOuterDeadZoneFraction() const
{
	return m_OuterDeadzoneFraction;
}


void AnalogJoystick::Reset()
{
	m_rawPosition = Vec2(0.f, 0.f);
	m_correctedPosition = Vec2(0.f, 0.f);
	m_InnerDeadzoneFraction = 0.15f;
	m_OuterDeadzoneFraction = 0.95f;
}


void AnalogJoystick::SetDeadZoneThresholds(float normalizedInnerDeadzoneThreshold, float normalizedOuterDeadzoneThreshold)
{
	m_InnerDeadzoneFraction = normalizedInnerDeadzoneThreshold;
	m_OuterDeadzoneFraction = normalizedOuterDeadzoneThreshold;
}


void AnalogJoystick::UpdatePosition(float rawNormalizedX, float rawNormalizedY)
{
	m_rawPosition.x = rawNormalizedX;
	m_rawPosition.y = rawNormalizedY;

	float theta = m_rawPosition.GetOrientationDegrees();
	float length = m_rawPosition.GetLength();
	//if (length < m_InnerDeadzoneFraction) length = m_InnerDeadzoneFraction;
	//if (length > m_OuterDeadzoneFraction) length = m_OuterDeadzoneFraction;
	float clampLength = RangeMapClamped(length, m_InnerDeadzoneFraction, m_OuterDeadzoneFraction, 0.f, 1.f);
	Vec2 correctedPos = Vec2::MakeFromPolarDegrees(theta, clampLength);
	m_correctedPosition.x = correctedPos.x;
	m_correctedPosition.y = correctedPos.y;
}


