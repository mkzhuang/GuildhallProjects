#pragma once
#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/OBB2.hpp"

class Capricorn : public Entity
{
public:
	Capricorn(Map* owner, Vec2 const& startPos, float orientation);
	~Capricorn() {}

	void Update(float deltaSeconds);
	void Render() const;

	void UpdateTransform(float deltaSeconds);
	void ReactToBulletHit(Bullet& bullet);
	void TakeDamage(int damage);
	void RenderEntity() const;
	void DebugRender() const;

private:
	Texture* m_bodyTexture = nullptr;
};


