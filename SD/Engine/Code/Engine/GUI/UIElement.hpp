#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/GUI/UICanvas.hpp"

#include <string>
#include <vector>

class Renderer;
class VertexBuffer;

class UIElement
{
public:
	UIElement();
	UIElement(std::string const& name, AABB2 const& bound, UIAnchor const& anchor, Renderer* renderer, bool isDraggable = false);
	virtual ~UIElement();

	virtual void Update() = 0;
	virtual void Render() const = 0;

	virtual void OnClick() = 0;

	void SetHover(bool isHover);
	void SetFocus(bool isFocus);
	bool IsHover() const;
	bool IsFocus() const;

	void SetColor(Rgba8 const& color);
	void SetFocusColor(Rgba8 const& color);
	void SetHoverColor(Rgba8 const& color);

	void AddChild(UIElement* child);

	void MoveWindow(Vec2 const& offset);
	AABB2 GetScreenWindow() const;
	Rgba8 GetHoverColor() const;
	Rgba8 GetFocusColor() const;
	std::string GetName() const;

public:
	std::string m_name = "default";
	UIAnchor m_anchor;
	AABB2 m_screenWindow = AABB2::ZERO_TO_ONE;
	UIElement* m_parent = nullptr;
	std::vector<UIElement*> m_children;
	bool m_isHover = false;
	bool m_isFocus = false;
	bool m_isDraggable = false;
	bool m_hasOwnCamera = false;

	Renderer* m_renderer = nullptr;
	std::vector<Vertex_PCU> m_verts;
	Rgba8 m_color = Rgba8::BLACK;
	Rgba8 m_hoverColor = Rgba8::CLEAR;
	Rgba8 m_focusColor = Rgba8::CLEAR;
	VertexBuffer* m_vertexBuffer = nullptr;
	bool m_isBufferDirty = true;
};