#pragma once
#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"

class Leo : public Entity
{
public:
	Leo(Map* owner, Vec2 const& startPos, float orientation);
	~Leo() {}

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


