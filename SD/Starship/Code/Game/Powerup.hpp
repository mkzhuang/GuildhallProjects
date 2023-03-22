#pragma once
#include "Game/Entity.hpp"

constexpr int NUM_POWERUP_TRIANGLES = 16;
constexpr int NUM_POWERUP_VERTICES = 3 * NUM_POWERUP_TRIANGLES;
constexpr float POWERUP_SEGEMENT_DEGREES = 360.f / static_cast<float>(NUM_POWERUP_TRIANGLES);

class Powerup : public Entity
{
public:
	Powerup(Game* owner, Vec2 const& startPos);
	~Powerup();

	void InitializeLocalVerts();
	void Update(float deltaSeconds);
	void Render() const;
	void Die();
	void WrapAround();

private:
	Vertex_PCU m_verts[NUM_POWERUP_VERTICES] = {};
	float m_timeCounter = 0.f;
};
