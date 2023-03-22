#include "Game/Aries.hpp"
#include "Engine/Math/OBB2.hpp"

Aries::Aries(Map* owner, Vec2 const& startPos, float orientation)
	: Entity(owner, startPos, orientation)
{
	m_faction = ENTITY_FACTION_EVIL;
	m_type = ENTITY_TYPE_EVIL_ARIES;
	m_maxHealth = g_gameConfigBlackboard.GetValue("enemyHealth", 0);
	m_health = g_gameConfigBlackboard.GetValue("enemyHealth", 0);
	m_randomGoalDelay = g_gameConfigBlackboard.GetValue("enemyWanderingDelay", 0.f);
	m_isActor = true;
	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_position = startPos;
	m_angularVelocity = g_gameConfigBlackboard.GetValue("enemyTurnRate", 0.f);
	m_physicsRadius = g_gameConfigBlackboard.GetValue("ariesPhysicsRadius", 0.f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("ariesCosmeticRadius", 0.f);
	m_bodyTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyAries.png");
}


void Aries::Update(float deltaSeconds)
{
	UpdateTransform(deltaSeconds);
}


void Aries::Render() const
{
	RenderEntity();
	if (g_isDebugging)
	{
		DebugRender();
	}
}


void Aries::UpdateTransform(float deltaSeconds)
{
	if (m_isPursuing) //pursuing
	{
		if (m_map->GetTileByPosition(m_lastTargetPosition) != m_map->GetTileByPosition(m_targetPosition))
		{
			IntVec2 targetCoords(RoundDownToInt(m_targetPosition.x), RoundDownToInt(m_targetPosition.y));
			m_map->PopulateDistanceFieldForEntityPathToGoal(m_distanceFieldToTarget, 999.f, this);
			m_pathPoints = m_map->GenerateEntityPathToGoal(m_distanceFieldToTarget, targetCoords);
			m_lastTargetPosition = m_targetPosition;
		}
		Entity::SetNextWayPoint();

		if (IsPointInsideDisc2D(m_targetPosition, m_position, m_physicsRadius))
		{
			m_isPursuing = false;
			m_pathPoints.clear();
			return;
		}

		Entity::SetNextWayPoint();
		Entity::MoveToNextWayPoint(deltaSeconds);
	}
	else // wandering
	{
		if (m_pathPoints.empty())
		{
			Entity::SetTargetPosition();
		}
		else
		{
			Entity::SetNextWayPoint();
			Entity::MoveToNextWayPoint(deltaSeconds);
		}
	}
}


void Aries::ReactToBulletHit(Bullet& bullet)
{
	static float reflectHalfAperture =  g_gameConfigBlackboard.GetValue("ariesReflectHalfAperture", 0.f);
	static float bulletRandomOffsetDegrees = g_gameConfigBlackboard.GetValue("bulletRandomOffsetDegrees", 0.f);

	Vec2 direction = (bullet.m_position - m_position).GetNormalized();
	if (GetAngleDegreesBetweenVectors2D(direction, GetForwardNormal()) < reflectHalfAperture)
	{
		PlaySound(g_soundIds[SOUND_TYPE_BULLET_RICOCHET], false, g_gameSoundVolume);
		PushDiscOutOfDisc2D(bullet.m_position, bullet.m_physicsRadius, m_position, m_physicsRadius);
		bullet.m_velocity.Reflect(direction);
		bullet.m_orientationDegrees = bullet.m_velocity.GetOrientationDegrees();
		bullet.m_orientationDegrees += RNG.RollRandomFloatInRange(-bulletRandomOffsetDegrees, bulletRandomOffsetDegrees);
		bullet.m_velocity.SetOrientationDegrees(bullet.m_orientationDegrees);
		bullet.TakeDamage(1);
	}
	else
	{
		Entity::ReactToBulletHit(bullet);
		PlaySound(g_soundIds[SOUND_TYPE_ENEMY_HIT], false, g_gameSoundVolume);
	}

}


void Aries::TakeDamage(int damage)
{
	Entity::TakeDamage(damage);
	if (m_health <= 0) 
	{
		float randomOrientation = RNG.RollRandomFloatInRange(0.f, 360.f);
		m_map->AddExplosionToMap(m_position, randomOrientation, ExplosionType::ENEMY_DEATH);
		PlaySound(g_soundIds[SOUND_TYPE_ENEMY_DIE], false, g_gameSoundVolume);
	}
}


void Aries::RenderEntity() const
{
	std::vector<Vertex_PCU> tankBaseVertexArray;
	OBB2 box(m_position, Vec2::MakeFromPolarDegrees(m_orientationDegrees), Vec2(0.5f, 0.5f));
	AddVertsForOBB2D(tankBaseVertexArray, box, Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(m_bodyTexture);
	g_theRenderer->DrawVertexArray(int(tankBaseVertexArray.size()), tankBaseVertexArray.data());
}


void Aries::DebugRender() const
{
	Entity::DebugRender();
}


