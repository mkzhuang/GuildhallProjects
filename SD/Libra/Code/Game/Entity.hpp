#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/HeatMaps.hpp"

#include <vector>

class Game;
class Map;
class Entity;
class Bullet;

enum EntityFaction
{
	ENTITY_FACTION_NULL = -1,

	ENTITY_FACTION_GOOD,
	ENTITY_FACTION_EVIL,
	ENTITY_FACTION_NEUTRAL,

	NUM_ENTITY_FACTIONS
};

enum EntityType
{
	ENTITY_TYPE_NULL = -1,

	ENTITY_TYPE_GOOD_PLAYER,
	ENTITY_TYPE_EVIL_SCORPIO, 
	ENTITY_TYPE_EVIL_LEO, 
	ENTITY_TYPE_EVIL_ARIES, 
	ENTITY_TYPE_EVIL_CAPRICORN,
	ENTITY_TYPE_GOOD_BULLET, 
	ENTITY_TYPE_GOOD_FLAME_BULLET,
	ENTITY_TYPE_EVIL_BULLET,
	ENTITY_TYPE_EVIL_MISSLE,
	
	NUM_ENTITY_TYPES
};

typedef std::vector<Entity*> EntityList;

class Entity
{
public:
	Entity(Map* owner, Vec2 const& startPos, float orientaiton = 0.f);
	virtual ~Entity() {}

	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;
	virtual void DebugRender() const = 0;
	virtual void ReactToBulletHit(Bullet& bullet);
	virtual void TakeDamage(int damage);
	virtual void SetTargetPosition();
	virtual void SetNextWayPoint();
	virtual void MoveToNextWayPoint(float deltaSeconds);
	void RenderHealth() const;
	void Die();
	Vec2 GetForwardNormal() const;

public:
	Map* m_map = nullptr;
	Vec2 m_position;
	Vec2 m_velocity;
	Vec2 m_targetPosition;
	Vec2 m_lastTargetPosition;
	std::vector<Vec2> m_pathPoints;
	Vec2 m_nextWayPoint;
	float m_orientationDegrees = 0.f;
	float m_turretOrientationDegrees = 0.f;
	float m_angularVelocity = 0.f;
	float m_turretAngularVelocity = 0.f;
	float m_targetOrientationDegrees = 0.f;
	float m_physicsRadius = 0.f;
	float m_cosmeticRadius = 0.f;
	float m_scale = 1.f;
	float m_shootDelay = 0.f;
	float m_shootTimer = 0.f;
	float m_randomGoalDelay = 0.f;
	float m_randomGoalTimer = 0.f;
	int m_health = 0;
	int m_maxHealth = 0;
	bool m_isDead = false;
	bool m_isGarbage = false;
	bool m_isPushedByEntities = false;
	bool m_doesPushEntities = false;
	bool m_isPushedByWalls = false;
	bool m_isHitByBullets = false;
	bool m_canSwim = false;
	bool m_isActor = false;
	bool m_isProjectile = false;
	bool m_isPursuing = false;
	bool m_hasSightOfPlayer = false;
	TileHeatMap m_distanceFieldToTarget;
	EntityType m_type = ENTITY_TYPE_NULL;
	EntityFaction m_faction = ENTITY_FACTION_NULL;
};


