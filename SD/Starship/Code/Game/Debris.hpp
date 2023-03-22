#pragma once
#include "Game/Entity.hpp"

constexpr int NUM_DEBRIS_TRIANGLES = 8;
constexpr int NUM_DEBRIS_VERTICES = 3 * NUM_DEBRIS_TRIANGLES;
constexpr float DEBRIS_SEGEMENT_DEGREES = 360.f / static_cast<float>(NUM_DEBRIS_TRIANGLES);

class Debris : public Entity
{
public:
	Debris(Game* owner, Vec2 const& startPos);
	~Debris();

	void Update(float deltaSeconds);
	void Render() const;
	void Die();
	void InitializeLocalVerts(Rgba8 color, float scale, Vec2 velocity);

private:
	Vertex_PCU m_verts[NUM_DEBRIS_VERTICES] = {};
};