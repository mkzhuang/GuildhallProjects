#include "Game/Bullet.hpp"
#include "Game/Player.hpp"
#include "Engine/Math/OBB2.hpp"

Bullet::Bullet(Map* owner, Vec2 const& startPos, float orientation, EntityFaction faction, EntityType type, bool isGuided)
	: Entity(owner, startPos, orientation)
	, m_isGuided(isGuided)
{
	m_faction = faction;
	m_type = type;
	m_isProjectile = true;
	m_isPushedByEntities = false;
	m_doesPushEntities = false;
	m_isPushedByWalls = false;
	m_isHitByBullets = false;
	m_position = startPos;
	m_physicsRadius = g_gameConfigBlackboard.GetValue("bulletPhysicsRadius", 0.f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("bulletCosmeticRadius", 0.f);

	if (m_isGuided)
	{
		m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees) * g_gameConfigBlackboard.GetValue("missleSpeed", 0.f);
		m_angularVelocity = g_gameConfigBlackboard.GetValue("missleTurnRate", 0.f);
	}
	else
	{
		m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees) * g_gameConfigBlackboard.GetValue("bulletSpeed", 0.f);
	}

	if (m_faction == ENTITY_FACTION_GOOD)
	{
		if (m_type == ENTITY_TYPE_GOOD_FLAME_BULLET) 
		{
			m_health = g_gameConfigBlackboard.GetValue("flameThrowerHealth", 1);
			m_scale = g_gameConfigBlackboard.GetValue("flameThrowerScale", 0.1f);
			float randomAngularMaxValue = g_gameConfigBlackboard.GetValue("flameThrowerRandomRotationDegrees", 0.f);
			m_angularVelocity = RNG.RollRandomFloatInRange(-randomAngularMaxValue, randomAngularMaxValue);
			float randomOffsetMaxValue = g_gameConfigBlackboard.GetValue("flameThrowerRandomOffsetDegrees", 0.f);
			float randomOffsetValue = RNG.RollRandomFloatInRange(-randomOffsetMaxValue, randomOffsetMaxValue);
			m_velocity.RotateDegrees(randomOffsetValue);
		}
		else
		{
			m_health = g_gameConfigBlackboard.GetValue("playerBulletHealth", 3);
			m_bodyTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/FriendlyBolt.png");
		}
	}
	else if (m_faction == ENTITY_FACTION_EVIL)
	{
		m_health = g_gameConfigBlackboard.GetValue("enemyBulletHealth", 1);
		m_bodyTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyBullet.png");
	}
	else
	{
		m_health = g_gameConfigBlackboard.GetValue("enemyBulletHealth", 1);
		m_bodyTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyBullet.png");
	}
}


void Bullet::Update(float deltaSeconds)
{
	UpdateTransform(deltaSeconds);
}


void Bullet::Render() const
{
	RenderEntity();
	if (g_isDebugging) DebugRender();
}


void Bullet::UpdateTransform(float deltaSeconds)
{
	Vec2 newPos = m_position + m_velocity * deltaSeconds;

	if (m_isGuided && m_map->IsPlayerAlive())
	{
		Vec2 displacement = m_map->GetPlayer()->m_position - m_position;
		m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, displacement.GetOrientationDegrees(), m_angularVelocity * deltaSeconds);
		m_velocity.SetOrientationDegrees(m_orientationDegrees);
	}

	if (m_type == ENTITY_TYPE_GOOD_FLAME_BULLET)
	{
		static float lifeTime = g_gameConfigBlackboard.GetValue("explosionLife", 0.f);
		m_timeAlive += deltaSeconds;
		if (m_timeAlive > lifeTime)
		{
			Die();
			return;
		}
		constexpr float velocityDecay = 0.97f;
		constexpr float sizeMultiplier = 1.75f;
		m_velocity *= velocityDecay;
		m_scale += (sizeMultiplier * deltaSeconds);
		m_cosmeticRadius *= m_scale;
		m_orientationDegrees += m_angularVelocity * deltaSeconds;
	}

	if (m_map->IsPointInSolid(newPos))
	{
		Tile* impactTile = m_map->GetTileByPosition(newPos);
		if (m_type == ENTITY_TYPE_GOOD_BULLET && impactTile->m_tileDef->m_isDestructible)
		{
 			impactTile->m_health--;
		}

		TakeDamage(1);
		float randomOrientation = RNG.RollRandomFloatInRange(0.f, 360.f);
		m_map->AddExplosionToMap(m_position, randomOrientation, ExplosionType::BULLET);
		if (m_health <= 0)
		{
			Die();
			return;
		}
		else
		{
			PlaySound(g_soundIds[SOUND_TYPE_BULLET_BOUNCE], false, g_gameSoundVolume);
			IntVec2 curTileCoords = m_map->GetTileByPosition(m_position)->m_tileCoords;
			IntVec2 newTileCoords = m_map->GetTileByPosition(newPos)->m_tileCoords;
			Vec2 bounceNormal = (Vec2(newTileCoords) - Vec2(curTileCoords)).GetNormalized();
			m_velocity.Reflect(bounceNormal);
			m_orientationDegrees = m_velocity.GetOrientationDegrees();
		}
	}
	else
	{
		m_position = newPos;
	}
}


void Bullet::RenderEntity() const
{
	if (m_type == ENTITY_TYPE_GOOD_FLAME_BULLET)
	{
		g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
		SpriteDefinition const& currentSpriteDef = g_exlosionAnimation->GetSpriteDefAtTime(m_timeAlive);
		std::vector<Vertex_PCU> verts;
		OBB2 box(m_position, Vec2::MakeFromPolarDegrees(m_orientationDegrees), Vec2(0.5f, 0.5f) * m_scale);
		AABB2 uvs = currentSpriteDef.GetUVs();
		AddVertsForOBB2D(verts, box, Rgba8::WHITE, uvs.m_mins, uvs.m_maxs);
		g_theRenderer->BindTexture(&currentSpriteDef.GetTexture());
		g_theRenderer->DrawVertexArray(int(verts.size()), verts.data());
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	}
	else
	{
		std::vector<Vertex_PCU> tankBaseVertexArray;
		OBB2 box(m_position, Vec2::MakeFromPolarDegrees(m_orientationDegrees), Vec2(0.1f, 0.05f));
		AddVertsForOBB2D(tankBaseVertexArray, box, Rgba8(255, 255, 255, 255));
		g_theRenderer->BindTexture(m_bodyTexture);
		g_theRenderer->DrawVertexArray(int(tankBaseVertexArray.size()), tankBaseVertexArray.data());
	}
}


void Bullet::DebugRender() const
{
	Entity::DebugRender();
}


