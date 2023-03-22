#include "Engine/Math/Capsule2.hpp"

Capsule2::Capsule2(Capsule2 const& copy)
	: m_bone(copy.m_bone)
	, m_radius(copy.m_radius)
{

}


Capsule2::Capsule2(LineSegment2 bone, float radius)
	: m_bone(bone)
	, m_radius(radius)
{

}


void Capsule2::Translate(Vec2 const& translation)
{
	m_bone.m_start += translation;
	m_bone.m_end += translation;
}


void Capsule2::SetCenter(Vec2 const& newCenter)
{
	Vec2 curCenter = m_bone.GetCenter();
	Vec2 translation = newCenter - curCenter;
	Translate(translation);
}


void Capsule2::RotateAboutCenter(float const& rotationDeltaDegrees)
{
	m_bone.RotateAboutCenter(rotationDeltaDegrees);
}


