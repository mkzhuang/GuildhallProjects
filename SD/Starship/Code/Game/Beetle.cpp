#include "Game/Beetle.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"

Beetle::Beetle(Game* owner, Vec2 const& startPos)
	:Entity(owner, startPos)
{
	m_physicsRadius = BEETLE_PHYSICS_RADIUS;
	m_cosmeticRadius = BEETLE_COSMETIC_RADIUS;
	health = BEETLE_HEALTH;
	maxHealth = BEETLE_HEALTH;
	m_mainColor = Rgba8(0, 175, 0, 255);
	InitializeLocalVerts();
}


Beetle::~Beetle()
{

}


void Beetle::Update(float deltaSeconds)
{
	PlayerShip* playerShip = m_game->GetNearestPlayerShip();
	if (!playerShip->m_isGarbage) {
		Vec2 playerShipPos = playerShip->m_position;
		Vec2 distanceVec = playerShipPos - m_position;
		m_orientationDegree = distanceVec.GetOrientationDegrees();
		m_velocity = distanceVec.GetNormalized() * BEETLE_SPEED;
	}
	m_position += (m_velocity * deltaSeconds);
}


void Beetle::Render() const
{
	Vertex_PCU tempBeetleVerts[NUM_BEETLE_VERTICES];
	for (int vertIndex = 0; vertIndex < NUM_BEETLE_VERTICES; vertIndex++)
	{
		tempBeetleVerts[vertIndex] = m_verts[vertIndex];
	}

	TransformVertexArrayXY3D(NUM_BEETLE_VERTICES, tempBeetleVerts, m_scale, m_orientationDegree, m_position);

	g_theRenderer->DrawVertexArray(NUM_BEETLE_VERTICES, tempBeetleVerts);
}


void Beetle::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}


void Beetle::InitializeLocalVerts()
{
	m_verts[0] = Vertex_PCU(Vec3(0.f, 0.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle A vertex 1
	m_verts[1] = Vertex_PCU(Vec3(-2.f, 1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle A vertex 2
	m_verts[2] = Vertex_PCU(Vec3(-2.f, -1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle A vertex 3

	m_verts[3] = Vertex_PCU(Vec3(-1.f, 2.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle B vertex 1
	m_verts[4] = Vertex_PCU(Vec3(-1.f, -1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle B vertex 2
	m_verts[5] = Vertex_PCU(Vec3(2.f, -1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle B vertex 3

	m_verts[6] = Vertex_PCU(Vec3(-1.f, -2.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle C vertex 1
	m_verts[7] = Vertex_PCU(Vec3(-1.f, 1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle C vertex 2
	m_verts[8] = Vertex_PCU(Vec3(2.f, 1.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle C vertex 3
}


