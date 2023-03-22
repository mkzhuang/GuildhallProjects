#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/AABB3.hpp"

#include <vector>

class Player : public Entity
{
public:
	Player(Game* owner, Vec3 const& startPos);
	~Player();

	void Update(float deltaSeconds);
	void Render() const;

	void KeepInBound(AABB3 const& bounds);

public:
	AABB3 m_bounds = AABB3::ZERO_TO_ONE;
	std::vector<Vertex_PCU> m_verts;
	float m_speed = 5.f;
	float m_mouseScale = 0.4f;
	float m_sprintMultiplier = 3.f;
};


