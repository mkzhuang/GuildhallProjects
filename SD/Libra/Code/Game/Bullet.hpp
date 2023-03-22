#pragma once
#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"

class Bullet : public Entity
{
public:
	Bullet(Map* owner, Vec2 const& startPos, float orientation, EntityFaction faction, EntityType type, bool isGuided = false);
	~Bullet() {}

	void Update(float deltaSeconds);
	void Render() const;

	void UpdateTransform(float deltaSeconds);
	void RenderEntity() const;
	void DebugRender() const;

private:
	Texture* m_bodyTexture = nullptr;
	bool m_isGuided = false;
	float m_timeAlive = 0.f;
};
