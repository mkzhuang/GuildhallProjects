#include "Game/Entity.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/World.hpp"
#include "Engine/Math/MathUtils.hpp"

Entity::Entity(World* owner, Vec3 const& startPos, EulerAngles const& orientation)
	: m_world(owner)
	, m_position(startPos)
	, m_orientationDegree(orientation)
{
}


void Entity::AddForce(Vec3 const& force)
{
	m_acceleration += force;
}


void Entity::AddImpulse(Vec3 const& impulse)
{
	m_velocity += impulse;
}


Mat44 Entity::GetModelMatrix() const
{
	Mat44 model;
	model.SetTranslation3D(m_position);
	Mat44 rotation = m_orientationDegree.GetAsMatrix_XFwd_YLeft_ZUp();
	model.Append(rotation);
	return model;
}


Vec3 Entity::GetEyePosition() const
{
	return m_bounds.GetCenterAtMinZ() + Vec3(0.f, 0.f, m_definition->m_eyeHeight);
}


Vec3 Entity::GetForwardNormal() const
{
	return m_orientationDegree.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
}


