#include "Engine/GUI/UIImage.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

constexpr float ONE_THIRD = 1.f / 3.f;
constexpr float TWO_THIRD = 2.f / 3.f;

UIImage::UIImage()
{
}


UIImage::UIImage(std::string const& name, AABB2 const& bound, UIAnchor const& anchor, Renderer* renderer, Texture* texture, bool isStretch, AABB2 const& textureUV, bool isDraggable, bool hasBorder)
	: UIElement(name, bound, anchor, renderer, isDraggable)
	, m_texture(texture)
	, m_isStretch(isStretch)
	, m_hasBorder(hasBorder)
	, m_textureUV(textureUV)
{
	if (m_texture)
	{
		m_color = Rgba8::WHITE;
	}
	else
	{
		m_color = Rgba8::CLEAR;
	}
}


UIImage::~UIImage()
{
}


void UIImage::Update()
{
	if (m_isBufferDirty)
	{
		m_verts.clear();

		if (!m_texture)
		{
			//AddVertsForHollowAABB2D(m_verts, m_screenWindow, 2.5f, m_color);
		}
		else
		if (m_isStretch)
		{
			Vec2 textureDimensions = (Vec2)m_texture->GetDimensions();
			Vec2 screenDimensions = m_screenWindow.GetDimensions();
			if (screenDimensions.x < textureDimensions.x || screenDimensions.y < textureDimensions.y)
			{
				AddVertsForAABB2D(m_verts, m_screenWindow, m_color, m_textureUV.m_mins, m_textureUV.m_maxs);
			}
			else
			{
				AABB2 UVs[9] = {};
				UVs[BottomLeft] = m_textureUV.GetBoxWithIn(AABB2(Vec2(0.f, 0.f), Vec2(ONE_THIRD, ONE_THIRD)));
				UVs[BottomCenter] = m_textureUV.GetBoxWithIn(AABB2(Vec2(ONE_THIRD, 0.f), Vec2(TWO_THIRD, ONE_THIRD)));
				UVs[BottomRight] = m_textureUV.GetBoxWithIn(AABB2(Vec2(TWO_THIRD, 0.f), Vec2(1.f, ONE_THIRD)));
				UVs[MiddleLeft] = m_textureUV.GetBoxWithIn(AABB2(Vec2(0.f, ONE_THIRD), Vec2(ONE_THIRD, TWO_THIRD)));
				UVs[MiddleCenter] = m_textureUV.GetBoxWithIn(AABB2(Vec2(ONE_THIRD, ONE_THIRD), Vec2(TWO_THIRD, TWO_THIRD)));
				UVs[MiddleRight] = m_textureUV.GetBoxWithIn(AABB2(Vec2(TWO_THIRD, ONE_THIRD), Vec2(1.f, TWO_THIRD)));
				UVs[TopLeft] = m_textureUV.GetBoxWithIn(AABB2(Vec2(0.f, TWO_THIRD), Vec2(ONE_THIRD, 1.f)));
				UVs[TopCenter] = m_textureUV.GetBoxWithIn(AABB2(Vec2(ONE_THIRD, TWO_THIRD), Vec2(TWO_THIRD, 1.f)));
				UVs[TopRight] = m_textureUV.GetBoxWithIn(AABB2(Vec2(TWO_THIRD, TWO_THIRD), Vec2(1.f, 1.f)));

				Vec2 dimensionsPerCell = textureDimensions / 3.f;
				float remainderWidth = screenDimensions.x - 2.f * dimensionsPerCell.x;
				float remainderHeight = screenDimensions.y - 2.f * dimensionsPerCell.y;

				int centerCountX = RoundDownToInt(remainderWidth / dimensionsPerCell.x);
				int centerCountY = RoundDownToInt(remainderHeight / dimensionsPerCell.y);
				float widthPerCenterCell = remainderWidth / static_cast<float>(centerCountX);
				float heightPerCenterCell = remainderHeight / static_cast<float>(centerCountY);

				Vec2 edgeRowDimensions(widthPerCenterCell, dimensionsPerCell.y);
				Vec2 edgeColumnDimensions(dimensionsPerCell.x, heightPerCenterCell);
				Vec2 centerDimensions(widthPerCenterCell, heightPerCenterCell);

				// four corners
				Vec2 start = m_screenWindow.m_mins;
				// bottom left
				AddVertsForAABB2D(m_verts, AABB2(start, start + dimensionsPerCell), m_color, UVs[BottomLeft].m_mins, UVs[BottomLeft].m_maxs);
				// bottom Right
				Vec2 bottomRightStart = start + Vec2(dimensionsPerCell.x + remainderWidth, 0.f);
				AddVertsForAABB2D(m_verts, AABB2(bottomRightStart, bottomRightStart + dimensionsPerCell), m_color, UVs[BottomRight].m_mins, UVs[BottomRight].m_maxs);
				// top left
				Vec2 topLeftStart = start + Vec2(0.f, dimensionsPerCell.y + remainderHeight);
				AddVertsForAABB2D(m_verts, AABB2(topLeftStart, topLeftStart + dimensionsPerCell), m_color, UVs[TopLeft].m_mins, UVs[TopLeft].m_maxs);
				// top right
				Vec2 topRightStart = topLeftStart + Vec2(dimensionsPerCell.x + remainderWidth, 0.f);
				AddVertsForAABB2D(m_verts, AABB2(topRightStart, topRightStart + dimensionsPerCell), m_color, UVs[TopRight].m_mins, UVs[TopRight].m_maxs);

				// bottom row
				Vec2 bottomStart = start + Vec2(dimensionsPerCell.x, 0.f);
				for (int index = 0; index < centerCountX; index++)
				{
					Vec2 mins = bottomStart + Vec2(widthPerCenterCell * static_cast<float>(index), 0.f);
					AddVertsForAABB2D(m_verts, AABB2(mins, mins + edgeRowDimensions), m_color, UVs[BottomCenter].m_mins, UVs[BottomCenter].m_maxs);
				}

				// top row
				Vec2 topStart = topLeftStart + Vec2(dimensionsPerCell.x, 0.f);
				for (int index = 0; index < centerCountX; index++)
				{
					Vec2 mins = topStart + Vec2(widthPerCenterCell * static_cast<float>(index), 0.f);
					AddVertsForAABB2D(m_verts, AABB2(mins, mins + edgeRowDimensions), m_color, UVs[TopCenter].m_mins, UVs[TopCenter].m_maxs);
				}

				// left column
				Vec2 leftStart = start + Vec2(0.f, dimensionsPerCell.y);
				for (int index = 0; index < centerCountY; index++)
				{
					Vec2 mins = leftStart + Vec2(0.f, heightPerCenterCell * static_cast<float>(index));
					AddVertsForAABB2D(m_verts, AABB2(mins, mins + edgeColumnDimensions), m_color, UVs[MiddleLeft].m_mins, UVs[MiddleLeft].m_maxs);
				}

				// right column
				Vec2 rightStart = bottomRightStart + Vec2(0.f, dimensionsPerCell.y);
				for (int index = 0; index < centerCountY; index++)
				{
					Vec2 mins = rightStart + Vec2(0.f, heightPerCenterCell * static_cast<float>(index));
					AddVertsForAABB2D(m_verts, AABB2(mins, mins + edgeColumnDimensions), m_color, UVs[MiddleRight].m_mins, UVs[MiddleRight].m_maxs);
				}

				// center
				Vec2 centerStart = start + dimensionsPerCell;
				for (int j = 0; j < centerCountY; j++)
				{
					for (int i = 0; i < centerCountX; i++)
					{
						Vec2 offset = Vec2(widthPerCenterCell * static_cast<float>(i), heightPerCenterCell * static_cast<float>(j));
						Vec2 mins = centerStart + offset;
						AddVertsForAABB2D(m_verts, AABB2(mins, mins + centerDimensions), m_color, UVs[MiddleCenter].m_mins, UVs[MiddleCenter].m_maxs);
					}
				}
			}
		}
		else
		{
			Vec2 const& center = m_screenWindow.GetCenter();
			Vec2 textureDimensions = (Vec2)m_texture->GetDimensions();
			AABB2 textureBox(Vec2::ZERO, textureDimensions);
			textureBox.SetCenter(center);
			AddVertsForAABB2D(m_verts, textureBox, m_color, m_textureUV.m_mins, m_textureUV.m_maxs);
		}

		m_renderer->CopyCPUToGPU(m_verts.data(), sizeof(Vertex_PCU) * m_verts.size(), m_vertexBuffer);
		m_isBufferDirty = false;
	}

	for (UIElement* child : m_children)
	{
		child->Update();
	}
}


void UIImage::Render() const
{
	if (m_texture)
	{
		m_renderer->BindTexture(m_texture);
		m_renderer->DrawVertexBuffer(m_vertexBuffer, (int)m_verts.size());
	}

	if (m_hasBorder)
	{
		std::vector<Vertex_PCU> borderVerts;
		AddVertsForHollowAABB2D(borderVerts, m_screenWindow, 2.5f, Rgba8::WHITE);
		m_renderer->BindTexture(nullptr);
		m_renderer->DrawVertexArray(borderVerts);
	}

	std::vector<Vertex_PCU> verts;
	if (m_isFocus)
	{
		if (m_parent) AddVertsForAABB2D(verts, GetScreenWindow(), m_parent->GetFocusColor());
	}
	if (m_isHover && !m_isFocus)
	{
		if(m_parent) AddVertsForAABB2D(verts, GetScreenWindow(), m_parent->GetHoverColor());
	}
	m_renderer->BindTexture(nullptr);
	m_renderer->DrawVertexArray(verts);

	for (UIElement* child : m_children)
	{
		if (child) child->Render();
	}
}


void UIImage::OnClick()
{
	if (m_parent) m_parent->OnClick();
}


void UIImage::SetTexture(Texture* texture)
{
	m_texture = texture;
	m_isBufferDirty = true;
}


