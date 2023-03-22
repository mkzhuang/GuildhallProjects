#include "Game/Leo.hpp"
#include "Engine/Math/OBB2.hpp"

Leo::Leo(Map* owner, Vec2 const& startPos, float orientation)
	: Entity(owner, startPos, orientation)
{
	m_faction = ENTITY_FACTION_EVIL;
	m_type = ENTITY_TYPE_EVIL_LEO;
	m_maxHealth = g_gameConfigBlackboard.GetValue("enemyHealth", 0);
	m_health = g_gameConfigBlackboard.GetValue("enemyHealth", 0);
	m_shootDelay = g_gameConfigBlackboard.GetValue("leoFireDelay", 0.f);
	m_shootTimer = m_shootDelay;
	m_randomGoalDelay = g_gameConfigBlackboard.GetValue("enemyWanderingDelay", 0.f);
	m_isActor = true;
	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_position = startPos;
	m_angularVelocity = g_gameConfigBlackboard.GetValue("enemyTurnRate", 0.f);
	m_physicsRadius = g_gameConfigBlackboard.GetValue("leoPhysicsRadius", 0.f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("leoCosmeticRadius", 0.f);
	m_bodyTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTank4.png");
}


void Leo::Update(float deltaSeconds)
{
	UpdateTransform(deltaSeconds);
}


void Leo::Render() const
{
	RenderEntity();
	if (g_isDebugging) 
	{
		DebugRender();
	}
}


void Leo::UpdateTransform(float deltaSeconds)
{
	static float bulletPhysicsRadius = g_gameConfigBlackboard.GetValue("bulletPhysicsRadius", 0.f);
	static float fireHalfAperture = g_gameConfigBlackboard.GetValue("enemyFireHalfAperture", 0.f);

	if (m_isPursuing)
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

		Vec2 targetDisplacement = m_targetPosition - m_position;
		float targetShootDegrees = GetAngleDegreesBetweenVectors2D(targetDisplacement, GetForwardNormal());
		if (targetShootDegrees < fireHalfAperture && m_hasSightOfPlayer)
		{
			m_shootTimer += deltaSeconds;
			if (m_shootTimer >= m_shootDelay)
			{
				PlaySound(g_soundIds[SOUND_TYPE_ENEMY_SHOOT], false, g_gameSoundVolume);
				Vec2 spawnPos = m_position + Vec2::MakeFromPolarDegrees(m_orientationDegrees) * (m_physicsRadius - bulletPhysicsRadius);
				Entity* newBullet = m_map->CreateEntityOfType(ENTITY_TYPE_EVIL_BULLET, spawnPos, m_orientationDegrees);
				m_map->AddEntityToMap(*newBullet);
				m_shootTimer = 0.f;
			}
		}
	}
	else
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


void Leo::ReactToBulletHit(Bullet& bullet)
{
	Entity::ReactToBulletHit(bullet);
	PlaySound(g_soundIds[SOUND_TYPE_ENEMY_HIT], false, g_gameSoundVolume);
}


void Leo::TakeDamage(int damage)
{
	Entity::TakeDamage(damage);
	if (m_health <= 0) 
	{
		float randomOrientation = RNG.RollRandomFloatInRange(0.f, 360.f);
		m_map->AddExplosionToMap(m_position, randomOrientation, ExplosionType::ENEMY_DEATH);
		PlaySound(g_soundIds[SOUND_TYPE_ENEMY_DIE], false, g_gameSoundVolume);
	}
}


void Leo::RenderEntity() const
{
	std::vector<Vertex_PCU> tankBaseVertexArray;
	OBB2 box(m_position, Vec2::MakeFromPolarDegrees(m_orientationDegrees), Vec2(0.5f, 0.5f));
	AddVertsForOBB2D(tankBaseVertexArray, box, Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(m_bodyTexture);
	g_theRenderer->DrawVertexArray(int(tankBaseVertexArray.size()), tankBaseVertexArray.data());
}


void Leo::DebugRender() const
{
	Entity::DebugRender();
}


