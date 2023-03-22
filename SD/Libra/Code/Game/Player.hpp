#pragma once
#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/OBB2.hpp"

class Player: public Entity
{
public:
	Player(Map* owner, Vec2 const& startPos, float orientation);
	~Player() {}

	void Update(float deltaSeconds);
	void Render() const;
	void DebugRender() const override;
	void RotateAndScale(float deltaSeconds, int direction, float scale);

private:
	void UpdateTransform(float deltaSeconds);
	void ReactToBulletHit(Bullet& bullet);
	void TakeDamage(int damage);
	void RenderEntity() const;
	void HandleKeyboardInput();
	void HandleControllerInput();
	void ShootBullet();

private:
	Texture* m_bodyTexture = nullptr;
	Texture* m_turretTexture = nullptr;
	Vec2 m_bodyMovementInput = Vec2::ZERO;
	Vec2 m_turretMovementInput = Vec2::ZERO;
	float m_targetBodyOrientation = 0.f;
	float m_targetTurretOrientation = 0.f;
	bool m_isUsingFlameThrower = false;
};


