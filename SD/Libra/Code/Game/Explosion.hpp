#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

enum class ExplosionType
{
	INVALID_TYPE = -1,

	BULLET,
	ENEMY_DEATH,
	PLAYER_DEATH
};

class Explosion : public Entity 
{
public:
	Explosion(Map* owner, Vec2 const& startPos, float orientation, ExplosionType explosionType);
	~Explosion() {}

	void Update(float deltaSeconds);
	void Render() const;
	void DebugRender() const override;

private:
	float m_timeAlive = 0.f;
	float m_playSpeed = 1.f;
	ExplosionType m_type = ExplosionType::INVALID_TYPE;
};