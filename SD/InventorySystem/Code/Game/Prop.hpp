#pragma once
#include "Game/Entity.hpp"
#include "Engine/Renderer/Texture.hpp"

#include <vector>

enum class PropType
{
	CUBE,
	SPHERE,
	GRID
};

class Prop : public Entity
{
public:
	Prop(Game* owner, Vec3 const& startPos, PropType type);
	~Prop();

	void Update(float deltaSeconds);
	void Render() const;
	void CreateCube();
	void CreateSphere();
	void CreateGrid();

public:
	std::vector<Vertex_PCU> m_verts;
	Texture* m_texture = nullptr;
	Rgba8 m_color = Rgba8::WHITE;
	PropType m_type;
};


