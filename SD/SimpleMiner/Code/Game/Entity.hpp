#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/EulerAngles.hpp"

constexpr float GRAVITY = -49.99f;

enum class GamePhysicsMode
{
	WALKING,
	FLYING,
	NOCLIP,

	NUM_MODES
};

class World;
class EntityDefinition;

class Entity
{
public:
	Entity(World* owner, Vec3 const& startPos, EulerAngles const& orientation);
	virtual ~Entity() {};

	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;
	
	void AddForce(Vec3 const& force);
	void AddImpulse(Vec3 const& impulse);
	Mat44 GetModelMatrix() const;
	Vec3 GetEyePosition() const;
	Vec3 GetForwardNormal() const;

public:
	World* m_world = nullptr;
	EntityDefinition const* m_definition = nullptr;
	GamePhysicsMode m_physicsMode = GamePhysicsMode::NOCLIP;
	AABB3 m_bounds;
	Vec3 m_position;
	Vec3 m_velocity;
	Vec3 m_acceleration;
	EulerAngles m_orientationDegree;
	EulerAngles m_angularVelocity;
};


