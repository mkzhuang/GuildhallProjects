#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

Entity::Entity(Game* owner, Vec2 const& startPos)
	: m_game(owner)
	, m_position(startPos)
{
}


void Entity::DebugRender() const
{
	Vec2 forward = GetForwardNormal() * m_cosmeticRadius;
	Vec2 left = forward.GetRotated90Degrees();
	DebugDrawLine(m_position, m_position + forward, 0.2f, Rgba8(255, 0, 0, 255));
	DebugDrawLine(m_position, m_position + left, 0.2f, Rgba8(0, 255, 0, 255));
	DebugDrawLine(m_position, m_position + m_velocity, 0.2f, Rgba8(255, 255, 0, 255));
	DebugDrawRing(m_position, m_cosmeticRadius * m_scale, 0.2f, Rgba8(255, 0, 255, 255));
	DebugDrawRing(m_position, m_physicsRadius * m_scale, 0.2f, Rgba8(0, 255, 255, 255));
}


void Entity::Die()
{
	m_isDead = true;
}


Vec2 Entity::GetForwardNormal() const
{
	return Vec2::MakeFromPolarDegrees(m_orientationDegree);
}


bool Entity::IsDead() const
{
	return m_isDead;
}


