#include "Game/Wasp.hpp"
#include "GameCommon.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"

Wasp::Wasp(Game* owner, Vec2 const& startPos)
	:Entity(owner, startPos)
{
	m_physicsRadius = WASP_PHYSICS_RADIUS;
	m_cosmeticRadius = WASP_COSMETIC_RADIUS;
	health = WASP_HEALTH;
	maxHealth = WASP_HEALTH;
	m_mainColor = Rgba8(255, 255, 0, 255);
	InitializeLocalVerts();
}


Wasp::~Wasp()
{

}


void Wasp::Update(float deltaSeconds)
{
	PlayerShip* playerShip = m_game->GetNearestPlayerShip();
	if (!playerShip->m_isGarbage) {
		Vec2 playerShipPos = playerShip->m_position;
		Vec2 distanceVec = playerShipPos - m_position;
		m_orientationDegree = distanceVec.GetOrientationDegrees();
		m_acceleration = distanceVec.GetNormalized() * WASP_ACCELERATION;
	}
	m_velocity += (m_acceleration * deltaSeconds);
	m_velocity.ClampLength(MAX_WASP_SPEED);
	m_position += (m_velocity * deltaSeconds);
}


void Wasp::Render() const
{
	Vertex_PCU tempWaspVerts[NUM_WASP_VERTICES];
	for (int vertIndex = 0; vertIndex < NUM_WASP_VERTICES; vertIndex++)
	{
		tempWaspVerts[vertIndex] = m_verts[vertIndex];
	}

	TransformVertexArrayXY3D(NUM_WASP_VERTICES, tempWaspVerts, m_scale, m_orientationDegree, m_position);

	g_theRenderer->DrawVertexArray(NUM_WASP_VERTICES, tempWaspVerts);
}


void Wasp::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}


void Wasp::InitializeLocalVerts()
{
	
	m_verts[0] = Vertex_PCU(Vec3(2.f, 0.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle A vertex 1
	m_verts[1] = Vertex_PCU(Vec3(0.5f, 1.5f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle A vertex 2
	m_verts[2] = Vertex_PCU(Vec3(0.5f, -1.5f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle A vertex 3

	m_verts[3] = Vertex_PCU(Vec3(2.f, 0.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle B vertex 1
	m_verts[4] = Vertex_PCU(Vec3(-1.5f, 1.5f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle B vertex 2
	m_verts[5] = Vertex_PCU(Vec3(-1.f, 0.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle B vertex 3

	m_verts[6] = Vertex_PCU(Vec3(2.f, 0.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle C vertex 1
	m_verts[7] = Vertex_PCU(Vec3(-1.5f, -1.5f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle C vertex 2
	m_verts[8] = Vertex_PCU(Vec3(-1.f, 0.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle C vertex 3

	m_verts[9] = Vertex_PCU(Vec3(0.f, .75f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle D vertex 1
	m_verts[10] = Vertex_PCU(Vec3(0.f, -.75f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle D vertex 2
	m_verts[11] = Vertex_PCU(Vec3(-2.f, 0.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle D vertex 3
}


