#pragma once
#include "Game/Entity.hpp"

constexpr int NUM_ASTEROID_TRIANGLES = 16;
constexpr int NUM_ASTEROID_VERTICES = 3 * NUM_ASTEROID_TRIANGLES;
constexpr float ASTEROID_SEGEMENT_DEGREES = 360.f / static_cast<float>(NUM_ASTEROID_TRIANGLES);

class Asteroid : public Entity
{
public:
	Asteroid(Game* owner, Vec2 const& startPos);
	~Asteroid();

	void InitializeLocalVerts();
	void Update(float deltaSeconds);
	void Render() const;
	void Die();
	void WrapAround();

private:
	Vertex_PCU m_verts[NUM_ASTEROID_VERTICES] = {};
};