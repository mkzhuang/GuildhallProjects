#include "Game/Entity.hpp"
#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

Entity::Entity(Map* owner, Vec2 const& startPos, float orientation)
	: m_map(owner)
	, m_position(startPos)
	, m_orientationDegrees(orientation)
	, m_distanceFieldToTarget(owner->GetMapDimensions())
{
}


void Entity::DebugRender() const
{
	Vec2 bodyForward = GetForwardNormal() * m_cosmeticRadius;
	Vec2 bodyLeft = bodyForward.GetRotated90Degrees();
	static float debugLineThickness = g_gameConfigBlackboard.GetValue("debugLineThickness", 0.f);
	DebugDrawRing(m_position, m_cosmeticRadius * m_scale, debugLineThickness, Rgba8(255, 0, 255, 255)); // draw cosmetic ring
	DebugDrawRing(m_position, m_physicsRadius * m_scale, debugLineThickness, Rgba8(0, 255, 255, 255)); // draw physics ring
	DebugDrawLine(m_position, m_position + bodyForward, debugLineThickness, Rgba8(255, 0, 0, 255)); // draw body forward vector
	DebugDrawLine(m_position, m_position + bodyLeft, debugLineThickness, Rgba8(0, 255, 0, 255)); // draw body left vector
	DebugDrawLine(m_position, m_position + m_velocity, debugLineThickness, Rgba8(255, 255, 0, 255)); // draw velocity vector

	if (m_type == ENTITY_TYPE_EVIL_BULLET || m_type == ENTITY_TYPE_EVIL_MISSLE || m_type == ENTITY_TYPE_GOOD_BULLET || m_type == ENTITY_TYPE_GOOD_FLAME_BULLET) 
	{
		return;
	}

	if (m_type != ENTITY_TYPE_EVIL_SCORPIO || m_hasSightOfPlayer) //draw destination debug line
	{
		DebugDrawLine(m_position, m_targetPosition, debugLineThickness, Rgba8(100, 100, 100, 255));
		DebugDrawDot(m_targetPosition, debugLineThickness * 2.f, Rgba8(100, 100, 100, 255));
	}

	if (m_type != ENTITY_TYPE_EVIL_SCORPIO) // draw next way point debug line
	{
		DebugDrawLine(m_position, m_nextWayPoint, debugLineThickness, Rgba8(0, 0, 100, 255));
		DebugDrawDot(m_nextWayPoint, debugLineThickness * 2.f, Rgba8(0, 0, 100, 255));
	}
}


void Entity::ReactToBulletHit(Bullet& bullet)
{
	if (m_isHitByBullets)
	{
		float randomOrientation = RNG.RollRandomFloatInRange(0.f, 360.f);
		m_map->AddExplosionToMap(bullet.m_position, randomOrientation, ExplosionType::BULLET);
		TakeDamage(1);
		bullet.TakeDamage(1);
	} 
}


void Entity::TakeDamage(int damage)
{
	m_health -= damage;
	if (m_health <= 0) 
	{
		Die();
	}
}


void Entity::SetTargetPosition()
{
	IntVec2 randomTargetLocation = m_map->RollSpawnLocation(!m_canSwim);
	m_map->PopulateDistanceFieldForEntityPathToGoal(m_distanceFieldToTarget, 999.f, this);
	while (m_distanceFieldToTarget.GetValue(randomTargetLocation) == 999.f)
	{
		randomTargetLocation = m_map->RollSpawnLocation(!m_canSwim);
		m_map->PopulateDistanceFieldForEntityPathToGoal(m_distanceFieldToTarget, 999.f, this);
	}

	m_pathPoints = m_map->GenerateEntityPathToGoal(m_distanceFieldToTarget, randomTargetLocation);
	m_targetPosition = Vec2(randomTargetLocation) + Vec2(0.5f, 0.5f);
	SetNextWayPoint();
}


void Entity::SetNextWayPoint()
{
	if (m_pathPoints.size() >= 2)
	{
		Vec2 nextTargetPosition = m_pathPoints[m_pathPoints.size() - 2];
		Vec2 entityLeft = GetForwardNormal().GetRotated90Degrees() * m_physicsRadius;
		bool isLeftRayClear = m_map->HasLineOfSight(m_position + entityLeft, nextTargetPosition + entityLeft);
		bool isRightRayClear = m_map->HasLineOfSight(m_position - entityLeft, nextTargetPosition - entityLeft);
		if (isLeftRayClear && isRightRayClear)
		{
			m_pathPoints.pop_back();
		}
	}
	
	if (IsPointInsideDisc2D(m_pathPoints.back(), m_position, m_physicsRadius))
	{
		m_pathPoints.pop_back();
	}

	if (m_pathPoints.size() > 0)
	{
		m_nextWayPoint = m_pathPoints.back();
	}
	else
	{
		SetTargetPosition();
	}
}


void Entity::MoveToNextWayPoint(float deltaSeconds)
{
	static float speed = g_gameConfigBlackboard.GetValue("enemySpeed", 0.f);
	static float driveHalfAperture = g_gameConfigBlackboard.GetValue("enemyDriveHalfAperture", 0.f);

	Vec2 displacement = m_nextWayPoint - m_position;
	if ((m_targetPosition - m_position).GetLengthSquared() <= 1.f) 
	{
		displacement = m_targetPosition - m_position;
	}
	m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, displacement.GetOrientationDegrees(), m_angularVelocity * deltaSeconds);
	float targetDegrees = GetAngleDegreesBetweenVectors2D(displacement, GetForwardNormal());
	m_velocity = GetForwardNormal() * speed;
	if (targetDegrees < driveHalfAperture)
	{
		m_position += m_velocity * deltaSeconds;
	}
}


void Entity::RenderHealth() const
{
	static Vec2 offset(-.4f, .5f);
	static float healthBarThickness = g_gameConfigBlackboard.GetValue("debugLineThickness", 0.f) * 5.f;
	float percentage = static_cast<float>(m_health) / static_cast<float>(m_maxHealth);
	Vec2 healthBarStartPos = m_position + offset;
	float healthBarLength = Interpolate(0.f, .8f, percentage);
	Vec2 healthBarEndPos = healthBarStartPos + Vec2(healthBarLength, 0.f);
	DebugDrawLine(healthBarStartPos, healthBarStartPos + Vec2(.8f, 0.f), healthBarThickness, Rgba8(255, 0, 0, 255));
	DebugDrawLine(healthBarStartPos, healthBarEndPos, healthBarThickness, Rgba8(0, 255, 0, 255));
}

void Entity::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}


Vec2 Entity::GetForwardNormal() const
{
	return Vec2::MakeFromPolarDegrees(m_orientationDegrees);
}


