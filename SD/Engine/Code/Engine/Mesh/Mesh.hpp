#pragma once
#include "Engine/Mesh/MeshBuilder.hpp"

class VertexBuffer;
class IndexBuffer;
class Renderer;
class Shader;
class Texture;

class Mesh
{
public:
	Mesh() {};
	Mesh(MeshBuilder const* meshBuilder, Renderer* renderer, std::string const& name = "");
	~Mesh();

	void RotateAboutZ(float degrees);
	void Rotate(Mat44& delta);
	void Rotate(Vec3 const& delta);
	Mat44 GetModelMatrix() const;
	void Render(Renderer* renderer) const;

public:
	std::string m_name = "";
	Mat44 m_modelMatrix;
	EulerAngles m_orientation = EulerAngles::ZERO;
	Mat44 m_orientationMatrix;
	VertexBuffer* m_vertexBuffer = nullptr;
	//IndexBuffer* m_indexBuffer = nullptr;
	int m_size = 0;
	Shader* m_shader = nullptr;
	Texture* m_texture = nullptr;
};


