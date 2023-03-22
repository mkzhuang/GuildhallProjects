#include "Engine/GUI/UIText.hpp"
#include "Engine/Renderer/Renderer.hpp"

UIText::UIText()
{
}


UIText::UIText(std::string const& name, AABB2 const& bound, UIAnchor const& anchor, Renderer* renderer, std::string const& text, BitmapFont* font, float textSize, Vec2 const& alignment, bool isDraggable)
	: UIElement(name, bound, anchor, renderer, isDraggable)
	, m_text(text)
	, m_font(font)
	, m_textSize(textSize)
	, m_alignment(alignment)
{
}


UIText::~UIText()
{
}


void UIText::Update()
{
	if (m_isBufferDirty)
	{
		m_verts.clear();
		m_font->AddVertsForTextInBox2D(m_verts, m_screenWindow, m_textSize, m_text, m_color, 0.6f, m_alignment);
		m_renderer->CopyCPUToGPU(m_verts.data(), sizeof(Vertex_PCU) * m_verts.size(), m_vertexBuffer);
		m_isBufferDirty = false;
	}

	for (UIElement* child : m_children)
	{
		child->Update();
	}
}


void UIText::Render() const
{
	std::vector<Vertex_PCU> verts;
	if (m_isFocus)
	{
		AddVertsForAABB2D(verts, GetScreenWindow(), GetFocusColor());
	}
	if (m_isHover && !m_isFocus)
	{
		AddVertsForAABB2D(verts, GetScreenWindow(), GetHoverColor());
	}
	m_renderer->BindTexture(nullptr);
	m_renderer->DrawVertexArray(verts);

	if (m_text != "")
	{
		m_renderer->BindTexture(&m_font->GetTexture());
		m_renderer->DrawVertexBuffer(m_vertexBuffer, (int)m_verts.size());
	}

	for (UIElement* child : m_children)
	{
		if (child) child->Render();
	}
}


void UIText::OnClick()
{
	if (m_parent) m_parent->OnClick();
}


void UIText::SetText(std::string const& text)
{
	m_text = text;
	m_isBufferDirty = true;
}


void UIText::SetTextSize(float textSize)
{
	m_textSize = textSize;
	m_isBufferDirty = true;
}


void UIText::SetAlignment(Vec2 const& alignment)
{
	m_alignment = alignment;
	m_isBufferDirty = true;
}


