#include "Engine/GUI/UIElement.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"

UIElement::UIElement()
{
	IntVec2 const& windowDimensions = Window::GetWindowContext()->GetClientDimensions();
	m_screenWindow = AABB2(Vec2::ZERO, Vec2(windowDimensions));
}


UIElement::UIElement(std::string const& name, AABB2 const& bound, UIAnchor const& anchor, Renderer* renderer, bool isDraggable)
	: m_name(name)
	, m_renderer(renderer)
	, m_isDraggable(isDraggable)
{
	AABB2 window = bound.GetBoxWithIn(anchor.m_fraction);
	window.Translate(anchor.m_offset);
	m_screenWindow = window;

	m_vertexBuffer = m_renderer->CreateVertexBuffer(sizeof(Vertex_PCU));
}


UIElement::~UIElement()
{
	delete m_vertexBuffer;
	for (UIElement* child : m_children)
	{
		delete child;
	}
}


//void UIElement::Update()
//{
//	if (m_isBufferDirty)
//	{
//		m_verts.clear();
//		AddVertsForAABB2D(m_verts, m_screenWindow, m_color);
//		m_renderer->CopyCPUToGPU(m_verts.data(), sizeof(Vertex_PCU) * m_verts.size(), m_vertexBuffer);
//		m_isBufferDirty = false;
//	}
//
//	for (UIElement* child : m_children)
//	{
//		child->Update();
//	}
//}


//void UIElement::Render() const
//{
//	m_renderer->BindTexture(nullptr);
//	m_renderer->DrawVertexBuffer(m_vertexBuffer, (int)m_verts.size());
//
//	for (UIElement* child : m_children)
//	{
//		child->Render();
//	}
//}


//void UIElement::OnClick()
//{
//	
//}


void UIElement::SetHover(bool isHover)
{
	m_isHover = isHover;
}


void UIElement::SetFocus(bool isFocus)
{
	m_isFocus = isFocus;
}


bool UIElement::IsHover() const
{
	return m_isHover;
}


bool UIElement::IsFocus() const
{
	return m_isFocus;
}


void UIElement::SetColor(Rgba8 const& color)
{
	m_color = color;
	m_isBufferDirty = true;
}


void UIElement::SetFocusColor(Rgba8 const& color)
{
	m_focusColor = color;
}


void UIElement::SetHoverColor(Rgba8 const& color)
{
	m_hoverColor = color;
}


void UIElement::AddChild(UIElement* child)
{
	m_children.push_back(child);
	child->m_parent = this;
}


void UIElement::MoveWindow(Vec2 const& offset)
{
	m_isBufferDirty = true;
	m_screenWindow.Translate(offset);
	for (UIElement* child : m_children)
	{
		child->MoveWindow(offset);
	}
}


AABB2 UIElement::GetScreenWindow() const
{
	return m_screenWindow;
}


Rgba8 UIElement::GetHoverColor() const
{
	return m_hoverColor;
}


Rgba8 UIElement::GetFocusColor() const
{
	return m_focusColor;
}


std::string UIElement::GetName() const
{
	return m_name;
}


