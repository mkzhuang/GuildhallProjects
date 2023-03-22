#include "Engine/GUI/UIScrollable.hpp"
#include "Engine/Renderer/Renderer.hpp"

UIScrollable::UIScrollable()
{
}


UIScrollable::UIScrollable(std::string const& name, AABB2 const& bound, UIAnchor const& anchor, Renderer* renderer, bool isDraggable, size_t totalEntries, size_t displayableEntries)
	: UIElement(name, bound, anchor, renderer, isDraggable)
	, m_totalEntries(totalEntries)
	, m_displayableEntries(displayableEntries)
{
}


UIScrollable::~UIScrollable()
{
}


void UIScrollable::Update()
{
	if (m_isBufferDirty)
	{
		m_verts.clear();

		float percentage = 1.f - (float)m_currentTopIndex / (float)m_totalEntries;
		float displayablePercent = (float)m_displayableEntries / (float)m_totalEntries;
		AABB2 scrollbarUVs(Vec2(0.1f, percentage - displayablePercent), Vec2(0.9f, percentage));
		AABB2 scrollbarBounds = m_screenWindow.GetBoxWithIn(scrollbarUVs);
		
		AddVertsForAABB2D(m_verts, scrollbarBounds, Rgba8(200, 200, 200, 255));

		m_renderer->CopyCPUToGPU(m_verts.data(), sizeof(Vertex_PCU) * m_verts.size(), m_vertexBuffer);
		m_isBufferDirty = false;
	}

	for (UIElement* child : m_children)
	{
		child->Update();
	}
}


void UIScrollable::Render() const
{
	if (m_displayableEntries >= m_totalEntries) return;

	m_renderer->BindTexture(nullptr);
	m_renderer->DrawVertexBuffer(m_vertexBuffer, (int)m_verts.size());

	for (UIElement* child : m_children)
	{
		if (child) child->Render();
	}
}


void UIScrollable::OnClick()
{

}


void UIScrollable::SetTotalEntries(size_t totalEntries)
{
	m_totalEntries = totalEntries;
	m_isBufferDirty = true;
}


void UIScrollable::SetDisplayableEntries(size_t displayableEntries)
{
	m_displayableEntries = displayableEntries;
	m_isBufferDirty = true;
}


bool UIScrollable::Scroll(bool down)
{
	if (m_displayableEntries >= m_totalEntries) return false;

	if (down)
	{
		size_t maxTopIndex = m_totalEntries - m_displayableEntries;
		if (m_currentTopIndex == maxTopIndex)
		{
			return false;
		}
		m_currentTopIndex++;
	}
	else
	{
		if (m_currentTopIndex == 0)
		{
			return false;
		}
		m_currentTopIndex--;
	}
	m_isBufferDirty = true;
	return true;
}


size_t const UIScrollable::GetCurrentTopIndex()
{
	return m_currentTopIndex;
}


