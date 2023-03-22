#pragma once
#include "Engine/GUI/GUI_Element.hpp"
#include "Engine/Mesh/MeshBuilder.hpp"
#include "Engine/Renderer/Camera.hpp"

class Mesh;

class GUI_ModelViewer : public GUI_Element
{
public:
	GUI_ModelViewer();
	~GUI_ModelViewer();

	void Update();
	void Render(Renderer* renderer) const;
	void RenderModel(Renderer* renderer) const;

	void SetupCamera();
	void SetMesh(MeshBuilderConfig config, Renderer* renderer);
	void SetMesh(Mesh* mesh);

public:
	Camera m_camera;
	Mesh* m_mesh = nullptr;
};