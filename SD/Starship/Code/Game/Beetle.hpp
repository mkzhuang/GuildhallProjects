#pragma once
#include "Game/Entity.hpp"

constexpr int NUM_BEETLE_TRIANGLES = 3;
constexpr int NUM_BEETLE_VERTICES = 3 * NUM_BEETLE_TRIANGLES;

class Beetle : public Entity
{
public:
	Beetle(Game* owner, Vec2 const&  startPos);
	~Beetle();

	void Update(float deltaSeconds);
	void Render() const;
	void Die();
	void InitializeLocalVerts();

private:
	Vertex_PCU m_verts[NUM_BEETLE_VERTICES] = {};
};