#pragma once
#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/OBB2.hpp"

class Scorpio : public Entity
{
public:
	Scorpio(Map* owner, Vec2 const& startPos, float orientation);
	~Scorpio() {}

	void Update(float deltaSeconds);
	void Render() const;

	void UpdateTransform(float deltaSeconds);
	void ReactToBulletHit(Bullet& bullet);
	void TakeDamage(int damage);
	void RenderEntity() const;
	void DebugRender() const;

public:
	Vec2 m_laserImpactPoint;

private:
	Texture* m_bodyTexture = nullptr;
	Texture* m_turretTexture = nullptr;
};