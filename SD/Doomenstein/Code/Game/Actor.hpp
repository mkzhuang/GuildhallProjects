#pragma once
#include "Game/Weapon.hpp"
#include "Game/ActorUID.hpp"
#include "Game/SpawnInfo.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"

class Map;
class ActorDefinition;
class Controller;
class AI;
class SpriteAnimationGroupDefinition;

enum Faction
{
	NEUTRAL,
	MARINE,
	DEMON,
	COUNT
};

enum AnimationType
{
	WALK,
	ATTACK,
	PAIN,
	DEATH
};

extern char const* g_factionNames[];

class Actor
{
public:
	Actor(Map* map, const SpawnInfo& spawnInfo);
	~Actor() {}

	void Update(float deltaSeconds);
	void UpdatePhysics(float deltaSeconds);
	void Render(Camera const& camera) const;
	Mat44 GetModelMatrix(Camera const& camera) const;
	Vec3 GetForward() const;
	Vec3 GetEyePosition() const;

	void Damage(float damage);
	void Shrink(float time);
	void Die();

	void AddForce(Vec3 const& force);
	void AddImpulse(Vec3 const& impulse);
	void OnCollide(Actor* other);
	void ChangeAnimationType(AnimationType type);

	void OnPossessed(Controller* controller);
	void OnUnpossessed(Controller* controller);
	void MoveInDirection(Vec3 direction, float speed);
	void TurnInDirection(EulerAngles direction);

	Weapon* GetEquippedWeapon();
	void EquipWeapon(int weaponIndex);
	void EquipNextWeapon();
	void EquipPreviousWeapon();
	void Attack();

public:
	ActorUID m_uid = ActorUID::INVALID;
	ActorDefinition const* m_definition = nullptr;
	Map *m_map = nullptr;

	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
	Vec3 m_velocity = Vec3::ZERO;
	EulerAngles m_angularVelocity = EulerAngles::ZERO;
	Vec3 m_acceleration = Vec3::ZERO;

	std::vector<Weapon*> m_weapons;
	int m_equippedWeaponIndex = -1;

	Actor* m_owner = nullptr;
	float m_shrinkScale = 0.25f;
	bool m_isShrinked = false;
	Stopwatch m_shirnkStopwatch;
	bool m_isDestroyed = false;
	bool m_isDead = false;

	float m_health = 200.0f;
	float m_animationTimer = 0.f;
	Stopwatch m_lifetimeStopwatch;
	SpriteAnimationGroupDefinition const* m_currentAnimationDef = nullptr;
	AnimationType m_animationType = AnimationType::WALK;

	Controller* m_controller = nullptr;
	AI* m_aiController = nullptr;

	bool m_renderForward = false;
	Rgba8 m_solidColor = Rgba8::RED;
	Rgba8 m_wireframeColor = Rgba8::WHITE;
};


