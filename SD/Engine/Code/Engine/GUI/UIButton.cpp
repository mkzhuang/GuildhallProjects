#include "Engine/GUI/UIButton.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EventSystem.hpp"

UIButton::UIButton()
{
}


UIButton::UIButton(std::string const& name, AABB2 const& bound, UIAnchor const& anchor, Renderer* renderer, bool isDraggable)
	: UIElement(name, bound, anchor, renderer, isDraggable)
{
	m_color = Rgba8::CLEAR;
	m_hoverColor = Rgba8(100, 100, 100, 50);
	m_focusColor = Rgba8(50, 50, 50, 150);
}


UIButton::~UIButton()
{
}


void UIButton::Update()
{
	if (m_isBufferDirty)
	{
		m_verts.clear();
		AddVertsForAABB2D(m_verts, m_screenWindow, m_color);
		m_renderer->CopyCPUToGPU(m_verts.data(), sizeof(Vertex_PCU) * m_verts.size(), m_vertexBuffer);
		m_isBufferDirty = false;
	}

	for (UIElement* child : m_children)
	{
		child->Update();
	}
}


void UIButton::Render() const
{
	m_renderer->BindTexture(nullptr);
	m_renderer->DrawVertexBuffer(m_vertexBuffer, (int)m_verts.size());

	for (UIElement* child : m_children)
	{
		if (child) child->Render();
	}
}


void UIButton::OnClick()
{
	CallHandler(m_handlerName);
}


void UIButton::SetHandler(std::string const& handlerName)
{
	m_handlerName = handlerName;
}


