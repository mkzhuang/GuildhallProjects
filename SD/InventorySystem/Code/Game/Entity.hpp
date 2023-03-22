#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

class Game;

class Entity
{
public:
	Entity(Game* owner, Vec3 const& startPos);
	virtual ~Entity() {};

	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;
	virtual void DebugRender() const;
	
	Mat44 GetModelMatrix() const;
	Vec3 GetForwardNormal() const;

public:
	Game* m_game = nullptr;
	Vec3 m_position;
	Vec3 m_velocity;
	EulerAngles m_orientationDegree;
	EulerAngles m_angularVelocity;
	float m_physicsRadius = 0.f;
	float m_cosmeticRadius = 0.f;
	float m_scale = 1.f;
};


