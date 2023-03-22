#include "Engine/Math/Capsule3.hpp"

Capsule3::Capsule3(LineSegment3 bone, float radius)
	: m_bone(bone)
	, m_radius(radius)
{
}


void Capsule3::Translate(Vec3 const& translation)
{
	m_bone.m_start += translation;
	m_bone.m_end += translation;
}


void Capsule3::SetCenter(Vec3 const& newCenter)
{
	Vec3 curCenter = m_bone.GetCenter();
	Vec3 translation = newCenter - curCenter;
	Translate(translation);
}


