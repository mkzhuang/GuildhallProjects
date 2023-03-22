#pragma once
#include "Engine/GUI/UIElement.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Mesh/MeshBuilder.hpp"

class Mesh;
class Texture;

class UI3DViewer : public UIElement
{
public:
	UI3DViewer();
	UI3DViewer(std::string const& name, AABB2 const& bound, UIAnchor const& anchor, Renderer* renderer, MeshBuilderConfig config, bool isDraggable = false);
	~UI3DViewer();

	void Update() override;
	void Render() const override;
	void OnClick() override;

	void RenderModel() const;

	void SetMesh(MeshBuilderConfig config);

public:
	Mesh* m_mesh = nullptr;
	Camera m_camera;
};


