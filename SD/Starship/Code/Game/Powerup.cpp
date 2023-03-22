#include "Game/Powerup.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"

#include <math.h>

Powerup::Powerup(Game* owner, Vec2 const& startPos)
	:Entity(owner, startPos)
{
	m_physicsRadius = POWERUP_PHYSICS_RADIUS;
	m_cosmeticRadius = POWERUP_COSMETIC_RADIUS;
	m_velocity = Vec2::MakeFromPolarDegrees(RNG.RollRandomFloatInRange(0.f, 360.f), 1.f) * POWERUP_SPEED;
	m_mainColor = Rgba8(200, 0, 0, 255);
	InitializeLocalVerts();
}


Powerup::~Powerup()
{

}


void Powerup::Update(float deltaSeconds)
{
	m_deathTimer += deltaSeconds;
	if (m_deathTimer >= POWERUP_LIFETIME_SECONDS)
	{
		Die();
		return;
	}

	m_position += m_velocity * deltaSeconds;
	WrapAround();

	m_timeCounter += deltaSeconds;
	m_mainColor.a = 200 - static_cast<unsigned char>(CosDegrees(m_timeCounter * 500.f) * 55.f);
	for (int triangleIndex = 0; triangleIndex < NUM_POWERUP_VERTICES; triangleIndex++)
	{
		m_verts[triangleIndex].m_color = m_mainColor;
	}
}


void Powerup::Render() const
{
	Vertex_PCU tempPowerupVerts[NUM_POWERUP_VERTICES];
	for (int vertIndex = 0; vertIndex < NUM_POWERUP_VERTICES; vertIndex++)
	{
		tempPowerupVerts[vertIndex] = m_verts[vertIndex];
	}

	TransformVertexArrayXY3D(NUM_POWERUP_VERTICES, tempPowerupVerts, m_scale, m_orientationDegree, m_position);

	g_theRenderer->DrawVertexArray(NUM_POWERUP_VERTICES, tempPowerupVerts);

	std::vector<Vertex_PCU> powerupTextVertexArray;
	AddVertsForTextTriangles2D(powerupTextVertexArray, "P", m_position - Vec2(.5f, 1.2f), 2.f, Rgba8(50, 50, 150, m_mainColor.a));
	g_theRenderer->DrawVertexArray(int(powerupTextVertexArray.size()), &powerupTextVertexArray[0]);
}


void Powerup::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}


void Powerup::InitializeLocalVerts()
{
	Vec2 firstVertPosition = Vec2(m_physicsRadius * CosDegrees(0.f),
		m_physicsRadius * SinDegrees(0.f));
	for (int triangleIndex = 0; triangleIndex < NUM_POWERUP_TRIANGLES; triangleIndex++)
	{
		Vec2 secondVertPosition = Vec2(m_physicsRadius * CosDegrees((triangleIndex + 1) * POWERUP_SEGEMENT_DEGREES),
			m_physicsRadius * SinDegrees((triangleIndex + 1) * POWERUP_SEGEMENT_DEGREES));
		m_verts[3 * triangleIndex] = Vertex_PCU(Vec3(0.f, 0.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle 1 vertex 1
		m_verts[3 * triangleIndex + 1] = Vertex_PCU(Vec3(firstVertPosition.x, firstVertPosition.y, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle 1 vertex 2
		m_verts[3 * triangleIndex + 2] = Vertex_PCU(Vec3(secondVertPosition.x, secondVertPosition.y, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle 1 vertex 3
		firstVertPosition = secondVertPosition;
	}
}


void Powerup::WrapAround()
{
	if (m_position.x < -m_cosmeticRadius) //off screen to left
	{
		m_position.x = WORLD_SIZE_X + m_cosmeticRadius;
	}
	if (m_position.x > WORLD_SIZE_X + m_cosmeticRadius) //off screen to right
	{
		m_position.x = -m_cosmeticRadius;
	}
	if (m_position.y < -m_cosmeticRadius) //off screen to bottom
	{
		m_position.y = WORLD_SIZE_Y + m_cosmeticRadius;
	}
	if (m_position.y > WORLD_SIZE_Y + m_cosmeticRadius) //off screen to top
	{
		m_position.y = -m_cosmeticRadius;
	}
}


