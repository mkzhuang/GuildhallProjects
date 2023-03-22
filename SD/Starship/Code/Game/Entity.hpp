#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

class Game;

class Entity
{
public:
	Entity(Game* owner, Vec2 const& startPos);
	virtual ~Entity();

	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;
	virtual void DebugRender() const;
	virtual void Die() = 0;

	virtual void CheckOutBound();
	bool IsOffscreen() const;
	Vec2 GetForwardNormal() const;
	bool IsAlive() const;

public:
	Game* m_game = nullptr;
	Vec2 m_position;
	Vec2 m_velocity;
	float m_orientationDegree = 0.f;
	float m_angularVelocity = 0.f;
	float m_physicsRadius = 0.f;
	float m_cosmeticRadius = 0.f;
	int health = 0;
	int maxHealth = 0;
	bool m_isDead = false;
	bool m_isGarbage = false;
	Rgba8 m_mainColor = Rgba8(0, 0, 0, 0);
	double m_deathTimer = 0.0;
	double m_timeLastFrame = 0.0;
	float m_scale = 1.f;
};