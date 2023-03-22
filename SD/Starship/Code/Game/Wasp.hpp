#pragma once
#include "Game/Entity.hpp"

constexpr int NUM_WASP_TRIANGLES = 4;
constexpr int NUM_WASP_VERTICES = 3 * NUM_WASP_TRIANGLES;

class Wasp : public Entity
{
public:
	Wasp(Game* owner, Vec2 const& startPos);
	~Wasp();

	void Update(float deltaSeconds);
	void Render() const;
	void Die();
	void InitializeLocalVerts();

public:
	Vec2 m_acceleration;

private:
	Vertex_PCU m_verts[NUM_WASP_VERTICES] = {};
	
};