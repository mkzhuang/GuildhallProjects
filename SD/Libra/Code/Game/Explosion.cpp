#include "Game/Explosion.hpp"

Explosion::Explosion(Map* owner, Vec2 const& startPos, float orientation, ExplosionType explosionType)
	: Entity(owner, startPos, orientation)
{
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("explosionCosmeticRadius", 0.f);
	m_type = explosionType;
	if (m_type == ExplosionType::BULLET)
	{
		m_scale = 0.2f;
		m_playSpeed = 5.f;
	}
	if (m_type == ExplosionType::PLAYER_DEATH)
	{
		m_scale = 3.f;
		m_playSpeed = 0.5f;
	}
	m_cosmeticRadius *= m_scale;
	
}


void Explosion::Update(float deltaSeconds)
{
	static float explosionLife = g_gameConfigBlackboard.GetValue("explosionLife", 0.f);
	m_timeAlive += deltaSeconds * m_playSpeed;
	if (m_timeAlive > explosionLife)
	{
		Entity::Die();
	}
}


void Explosion::Render() const
{
	SpriteDefinition const& currentSpriteDef = g_exlosionAnimation->GetSpriteDefAtTime(m_timeAlive);
	std::vector<Vertex_PCU> verts;
	OBB2 box(m_position, Vec2::MakeFromPolarDegrees(m_orientationDegrees), Vec2(0.5f, 0.5f) * m_scale);
	AABB2 uvs = currentSpriteDef.GetUVs();
	AddVertsForOBB2D(verts, box, Rgba8::WHITE, uvs.m_mins, uvs.m_maxs);
	g_theRenderer->BindTexture(&currentSpriteDef.GetTexture());
	g_theRenderer->DrawVertexArray(int(verts.size()), verts.data());
}


void Explosion::DebugRender() const
{
	float debugLineThickness = g_gameConfigBlackboard.GetValue("debugLineThickness", 0.f);
	DebugDrawRing(m_position, m_cosmeticRadius * m_scale, debugLineThickness, Rgba8(255, 0, 255, 255)); // draw cosmetic ring
}


