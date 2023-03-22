#pragma once
#include "Game/Entity.hpp"

constexpr int NUM_BULLET_TRIANGLES = 2;
constexpr int NUM_BULLET_VERTICES = 3 * NUM_BULLET_TRIANGLES;

class Bullet : public Entity
{
public:
	Bullet(Game* owner, Vec2 const& startPos);
	~Bullet();

	void InitializeLocalVerts();
	void Update(float deltaSeconds);
	void Render() const;
	void Die();

private:
	Vertex_PCU m_verts[NUM_BULLET_VERTICES] = {};
};