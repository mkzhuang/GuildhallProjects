#include "Game/Debris.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Time.hpp"

#include <math.h>

extern RandomNumberGenerator RNG;

Debris::Debris(Game* owner, Vec2 const& startPos)
	:Entity(owner, startPos)
{
	m_physicsRadius = DEBRIS_PHYSICAL_RADIUS;
	m_cosmeticRadius = DEBRIS_COSMETIC_RADIUS;
}


Debris::~Debris()
{

}


void Debris::Update(float deltaSeconds)
{
	m_deathTimer += deltaSeconds;
	if (m_deathTimer >= DEBRIS_LIFE_TIME)
	{
		Die();
		return;
	}
		
	float transparentRatio = 1.f - static_cast<float>(m_deathTimer / DEBRIS_LIFE_TIME);
	unsigned char currentAlpha = static_cast<unsigned char>(transparentRatio * DEBRIS_DEFAULT_ALPHA);
	for (int vertIndex = 0; vertIndex < NUM_DEBRIS_VERTICES; vertIndex++)
	{
		m_verts[vertIndex].m_color.a = currentAlpha;
	}

	m_position += (m_velocity * deltaSeconds);
	m_orientationDegree += (m_angularVelocity * deltaSeconds);
}


void Debris::Render() const
{
	Vertex_PCU tempDebrisVerts[NUM_DEBRIS_VERTICES];
	for (int vertIndex = 0; vertIndex < NUM_DEBRIS_VERTICES; vertIndex++)
	{
		tempDebrisVerts[vertIndex] = m_verts[vertIndex];
	}

	TransformVertexArrayXY3D(NUM_DEBRIS_VERTICES, tempDebrisVerts, m_scale, m_orientationDegree, m_position);

	g_theRenderer->DrawVertexArray(NUM_DEBRIS_VERTICES, tempDebrisVerts);
}


void Debris::Die()
{
	m_isGarbage = true;
	m_isDead = true;
}


void Debris::InitializeLocalVerts(Rgba8 color, float scale, Vec2 velocity)
{
	m_mainColor = color;
	m_scale = scale;
	m_velocity = velocity;

	float randomFirstVertLength = RNG.RollRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
	Vec2 firstVertPosition = Vec2(randomFirstVertLength * CosDegrees(0.f),
								  randomFirstVertLength * SinDegrees(0.f));
	for (int triangleIndex = 0; triangleIndex < NUM_DEBRIS_TRIANGLES; triangleIndex++)
	{
		float randomSecondVertLength = RNG.RollRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
		Vec2 secondVertPosition = Vec2(randomSecondVertLength * CosDegrees((triangleIndex + 1) * DEBRIS_SEGEMENT_DEGREES),
			randomSecondVertLength * SinDegrees((triangleIndex + 1) * DEBRIS_SEGEMENT_DEGREES));
		m_verts[3 * triangleIndex] = Vertex_PCU(Vec3(0.f, 0.f, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle 1 vertex 1
		m_verts[3 * triangleIndex + 1] = Vertex_PCU(Vec3(firstVertPosition.x, firstVertPosition.y, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle 1 vertex 2
		m_verts[3 * triangleIndex + 2] = Vertex_PCU(Vec3(secondVertPosition.x, secondVertPosition.y, 0.f), m_mainColor, Vec2(0.f, 0.f)); //triangle 1 vertex 3
		firstVertPosition = secondVertPosition;
	}
}


