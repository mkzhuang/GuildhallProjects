#include "Game/Scorpio.hpp"

static float enemySightDistance;

Scorpio::Scorpio(Map* owner, Vec2 const& startPos, float orientation)
	: Entity(owner, startPos, orientation)
{
	m_faction = ENTITY_FACTION_EVIL;
	m_type = ENTITY_TYPE_EVIL_SCORPIO;
	m_maxHealth = g_gameConfigBlackboard.GetValue("enemyHealth", 0);
	m_health = g_gameConfigBlackboard.GetValue("enemyHealth", 0);
	m_shootDelay = g_gameConfigBlackboard.GetValue("scorpioFireDelay", 0.f);
	m_shootTimer = m_shootDelay;
	m_isActor = true;
	m_isPushedByEntities = false;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_position = startPos;
	m_angularVelocity = g_gameConfigBlackboard.GetValue("scorpioTurretTurnRate", 0.f);
	m_physicsRadius = g_gameConfigBlackboard.GetValue("scorpioPhysicsRadius", 0.f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("scorpioCosmeticRadius", 0.f);
	m_bodyTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTurretBase.png");
	m_turretTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyCannon.png");

	enemySightDistance = g_gameConfigBlackboard.GetValue("enemySightDistance", 0.f);
}


void Scorpio::Update(float deltaSeconds)
{
	UpdateTransform(deltaSeconds);
}


void Scorpio::Render() const
{
	RenderEntity();
	if (g_isDebugging) 
	{
		DebugRender();
	}
}


void Scorpio::UpdateTransform(float deltaSeconds)
{
	static float fireHalfAperture = g_gameConfigBlackboard.GetValue("enemyFireHalfAperture", 0.f);
	static float bulletPhysicsRadius = g_gameConfigBlackboard.GetValue("bulletPhysicsRadius", 0.f);

	if (m_isPursuing) // pursuing
	{
		Vec2 displacement = m_targetPosition - m_position;
		m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, displacement.GetOrientationDegrees(), m_angularVelocity * deltaSeconds);
		float targetDegrees = GetAngleDegreesBetweenVectors2D(displacement, GetForwardNormal());
		if (targetDegrees < fireHalfAperture)
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
	else // wandering
	{
		m_orientationDegrees += m_angularVelocity * deltaSeconds;
		m_laserImpactPoint = m_position + Vec2::MakeFromPolarDegrees(m_orientationDegrees) * enemySightDistance;
	}
}


void Scorpio::ReactToBulletHit(Bullet& bullet)
{
	Entity::ReactToBulletHit(bullet);
	PlaySound(g_soundIds[SOUND_TYPE_ENEMY_HIT], false, g_gameSoundVolume);
}


void Scorpio::TakeDamage(int damage)
{
	Entity::TakeDamage(damage);
	if (m_health <= 0)
	{
		float randomOrientation = RNG.RollRandomFloatInRange(0.f, 360.f);
		m_map->AddExplosionToMap(m_position, randomOrientation, ExplosionType::ENEMY_DEATH);
		PlaySound(g_soundIds[SOUND_TYPE_ENEMY_DIE], false, g_gameSoundVolume);
	}
}


void Scorpio::RenderEntity() const
{
	float debugLineThickness = g_gameConfigBlackboard.GetValue("debugLineThickness", 0.f);
	DrawLaser(m_position, m_laserImpactPoint, debugLineThickness, enemySightDistance, Rgba8(255, 0, 0, 255));

	std::vector<Vertex_PCU> tankBaseVertexArray;
	AABB2 bodyBox(m_position - Vec2(0.5f, 0.5f), m_position + Vec2(0.5f, 0.5f));
	AddVertsForAABB2D(tankBaseVertexArray, bodyBox, Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(m_bodyTexture);
	g_theRenderer->DrawVertexArray(int(tankBaseVertexArray.size()), tankBaseVertexArray.data());

	std::vector<Vertex_PCU> tankTopVertexArray;
	OBB2 turretBox(m_position, Vec2::MakeFromPolarDegrees(m_orientationDegrees), Vec2(0.5f, 0.5f));
	AddVertsForOBB2D(tankTopVertexArray, turretBox, Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(m_turretTexture);
	g_theRenderer->DrawVertexArray(int(tankTopVertexArray.size()), tankTopVertexArray.data());
}


void Scorpio::DebugRender() const
{
	Entity::DebugRender();
}


