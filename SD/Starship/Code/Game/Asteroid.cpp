#include "Game/Asteroid.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

#include <math.h>

extern RandomNumberGenerator RNG;

Asteroid::Asteroid(Game* owner, Vec2 const& startPos)
	:Entity(owner, startPos)
{
	m_physicsRadius = ASTEROID_PHYSICS_RADIUS;
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS;
	health = ASTEROID_HEALTH;
	maxHealth = ASTEROID_HEALTH;
	m_mainColor = Rgba8(100, 100, 100, 255);
	InitializeLocalVerts();
}


Asteroid::~Asteroid()
{

}


void Asteroid::Update(float deltaSeconds)
{
	m_position += (m_velocity * deltaSeconds);
	m_orientationDegree += (m_angularVelocity * deltaSeconds);
	WrapAround();
}


void Asteroid::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}


void Asteroid::Render() const
{
	Vertex_PCU tempAsteroidVerts[NUM_ASTEROID_VERTICES];
	for (int vertIndex = 0; vertIndex < NUM_ASTEROID_VERTICES; vertIndex++)
	{
		tempAsteroidVerts[vertIndex] = m_verts[vertIndex];
	}

	TransformVertexArrayXY3D(NUM_ASTEROID_VERTICES, tempAsteroidVerts, m_scale, m_orientationDegree, m_position);

	g_theRenderer->DrawVertexArray(NUM_ASTEROID_VERTICES, tempAsteroidVerts);
}


void Asteroid::InitializeLocalVerts()
{
	
	float randomFirstVertLength = RNG.RollRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
	Vec2 firstVertPosition = Vec2(randomFirstVertLength * CosDegrees(0.f), 
								  randomFirstVertLength * SinDegrees(0.f));
	for (int triangleIndex = 0; triangleIndex < NUM_ASTEROID_TRIANGLES; triangleIndex++)
	{
		float randomSecondVertLength = RNG.RollRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
		Vec2 secondVertPosition = Vec2(randomSecondVertLength * CosDegrees((triangleIndex + 1) * ASTEROID_SEGEMENT_DEGREES), 
			randomSecondVertLength * SinDegrees((triangleIndex + 1) * ASTEROID_SEGEMENT_DEGREES));
		m_verts[3 * triangleIndex] = Vertex_PCU(Vec3(0.f, 0.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle 1 vertex 1
		m_verts[3 * triangleIndex + 1] = Vertex_PCU(Vec3(firstVertPosition.x, firstVertPosition.y, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle 1 vertex 2
		m_verts[3 * triangleIndex + 2] = Vertex_PCU(Vec3(secondVertPosition.x, secondVertPosition.y, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle 1 vertex 3
		firstVertPosition = secondVertPosition;
	}

}


void Asteroid::WrapAround()
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


