#pragma once
#include "Game/Entity.hpp"

class Game;
class XboxController;

constexpr int NUM_SHIP_TRIANGLES = 6;
constexpr int NUM_SHIP_VERTICES = 3 * NUM_SHIP_TRIANGLES;

class PlayerShip : public Entity
{
public:
	PlayerShip(Game* owner, Vec2 const& startPos);
	~PlayerShip();

	void InitializeLocalVerts();
	void Update(float deltaSeconds);
	void Render() const;
	void Die();

private:
	void HandleKeyPressed(float deltaSeconds);
	void HandleControllerInput(float deltaSeconds);
	void BounceBackWhenOutbounds();
	void Respawn();
	void FireBullet();
	
public:
	int m_bulletCount = 1;

private:
	Vertex_PCU m_verts[NUM_SHIP_VERTICES] = {};
	float m_thrustFraction = 0.f;
	Rgba8 m_flameColor = Rgba8(200, 0, 0, 255);
};