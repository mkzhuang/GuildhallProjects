#include "Game/Bullet.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"


Bullet::Bullet(Game* owner, Vec2 const& startPos)
	: Entity(owner, startPos)
{
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_mainColor = Rgba8(255, 255, 0, 255);
	InitializeLocalVerts();
}


Bullet::~Bullet()
{

}


void Bullet::InitializeLocalVerts()
{
	m_verts[0] = Vertex_PCU(Vec3(0.f, 0.5f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle A vertex 1
	m_verts[1] = Vertex_PCU(Vec3(0.5f, 0.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle A vertex 2
	m_verts[2] = Vertex_PCU(Vec3(0.f, -0.5f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle A vertex 3

	Rgba8 bulletTailColor = Rgba8(255, 0, 0, 255);
	m_verts[3] = Vertex_PCU(Vec3(0.f, -0.5f, 0.f), bulletTailColor, Vec2(0.f, 0.f)); //triangle B vertex 1
	m_verts[4] = Vertex_PCU(Vec3(0.f, 0.5f, 0.f), bulletTailColor, Vec2(0.f, 0.f)); //triangle B vertex 2
	Rgba8 bulletFadeColor = Rgba8(255, 0, 0, 0);
	m_verts[5] = Vertex_PCU(Vec3(-2.f, 0.f, 0.f), bulletFadeColor, Vec2(0.f, 0.f)); //triangle B vertex 3
}


void Bullet::Update(float deltaSeconds)
{
	m_deathTimer += deltaSeconds;
	if (m_deathTimer >= DEBRIS_LIFE_TIME)
	{
		Die();
		return;
	}

	m_position += (m_velocity * deltaSeconds);
	m_orientationDegree += (m_angularVelocity * deltaSeconds);
}


void Bullet::Render() const
{
	Vertex_PCU tempBulletVerts[NUM_BULLET_VERTICES];
	for (int vertIndex = 0; vertIndex < NUM_BULLET_VERTICES; vertIndex++)
	{
		tempBulletVerts[vertIndex] = m_verts[vertIndex];
	}

	TransformVertexArrayXY3D(NUM_BULLET_VERTICES, tempBulletVerts, m_scale, m_orientationDegree, m_position);

	g_theRenderer->DrawVertexArray(NUM_BULLET_VERTICES, tempBulletVerts);
}


void Bullet::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}


