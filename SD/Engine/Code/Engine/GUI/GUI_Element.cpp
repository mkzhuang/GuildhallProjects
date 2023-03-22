#include "Engine/GUI/GUI_Element.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/Clock.hpp"

GUI_Element::GUI_Element()
{
}


GUI_Element::~GUI_Element()
{
	m_borderVerts.clear();
	m_backgroundVerts.clear();
	for (GUI_Element* child : m_children)
	{
		delete child;
	}
	delete m_borderCBO;
	m_borderCBO = nullptr;
}


void GUI_Element::Update()
{
	if (m_isHover)
	{
		m_animationTimer += (float)Clock::GetSystemClock().GetDeltaTime();
	}
	else
	{
		m_animationTimer -= (float)Clock::GetSystemClock().GetDeltaTime();
	}
	m_animationTimer = Clamp(m_animationTimer, 0.f, m_animationDuration);

	if (m_animationTimer > 0.f)
	{
		CreateAnimationVerts();
	}
	
	if (m_isBufferDirty)
	{
		ReconstructElement();
		m_isBufferDirty = false;
	}
}


void GUI_Element::Render(Renderer* renderer) const
{
	renderer->BindTexture(nullptr);
	renderer->DrawVertexArray(m_borderVerts);

	if (m_borderStyle == GUI_BorderStyle::ROUNDED_NONE || m_borderStyle == GUI_BorderStyle::ROUNDED || m_borderStyle == GUI_BorderStyle::DOUBLE_ROUNDED)
	{
		const_cast<GUI_Element*>(this)->SetConstantBuffer();
		const_cast<GUI_Element*>(this)->BindConstantBuffer(renderer);
		Shader* shader = renderer->CreateOrGetShader("Data/Shaders/RoundedBackground");
		renderer->BindShader(shader);
	}

	if (m_animationStyle != GUI_AnimationStyle::NONE)
	{
		RenderAnimation(renderer);
	}

	if (!m_isFocus && !m_isHover)
	{
		renderer->BindTexture(m_backgroundTexture);
		renderer->DrawVertexArray(m_backgroundVerts);
	}

	if (m_isFocus)
	{
		RenderFocus(renderer);
	}

	if (!m_isFocus && m_isHover)
	{
		RenderHover(renderer);
	}

	renderer->BindShader(nullptr);
}


void GUI_Element::RenderHover(Renderer* renderer) const
{
	renderer->BindTexture(m_hoverTexture);
	renderer->DrawVertexArray(m_hoverVerts);
}


void GUI_Element::RenderFocus(Renderer* renderer) const
{
	renderer->BindTexture(m_focusTexture);
	renderer->DrawVertexArray(m_focusVerts);
}


void GUI_Element::RenderAnimation(Renderer* renderer) const
{
	if (m_animationTimer == 0.f) return; 
	renderer->BindTexture(nullptr);
	renderer->DrawVertexArray(m_animationVerts);
}


void GUI_Element::SetCanvas(GUI_Canvas* canvas)
{
	m_canvas = canvas;
}

void GUI_Element::SetName(std::string const& name)
{
	m_name = name;
}


void GUI_Element::SetBounds(AABB2 const& bounds)
{
	m_elementWindow = bounds;
	m_originPosition = m_elementWindow.GetPointAtUV(Vec2(0.5f, 0.5f));
	SetBufferDirty();
}


void GUI_Element::SetSize(Vec2 const& size)
{
	m_size = size;
	SetBufferDirty();
}


void GUI_Element::SetAnchorType(GUI_AnchorType anchorType)
{
	m_anchorType = anchorType;
	AABB2 parentWindow;
	if (m_parent)
	{
		parentWindow = m_parent->m_elementWindow;
	}
	else
	{
		IntVec2 const& windowDimensions = Window::GetWindowContext()->GetClientDimensions();
		float windowSizeX = static_cast<float>(windowDimensions.x);
		float windowSizeY = static_cast<float>(windowDimensions.y);
		parentWindow = AABB2(Vec2::ZERO, Vec2(windowSizeX, windowSizeY));
	}
	Vec2 center = m_elementWindow.GetPointAtUV(Vec2(0.5f, 0.5f));
	Vec2 offset;
	switch (m_anchorType)
	{
		case GUI_AnchorType::TOP_LEFT:
			m_anchorPosition = center - parentWindow.GetPointAtUV(Vec2(0.f, 1.f));
			break;
		case GUI_AnchorType::TOP_CENTER:
			m_anchorPosition = center - parentWindow.GetPointAtUV(Vec2(0.5f, 1.f));
			break;
		case GUI_AnchorType::TOP_RIGHT:
			m_anchorPosition = center - parentWindow.GetPointAtUV(Vec2(1.f, 1.f));
			break;
		case GUI_AnchorType::MIDDLE_LEFT:
			m_anchorPosition = center - parentWindow.GetPointAtUV(Vec2(0.f, 0.5f));
			break;
		case GUI_AnchorType::CENTER:
			m_anchorPosition = center - parentWindow.GetPointAtUV(Vec2(0.5f, 0.5f));
			break;
		case GUI_AnchorType::MIDDLE_RIGHT:
			m_anchorPosition = center - parentWindow.GetPointAtUV(Vec2(1.f, 0.5f));
			break;
		case GUI_AnchorType::BOTTOM_LEFT:
			m_anchorPosition = center - parentWindow.GetPointAtUV(Vec2(0.f, 0.f));
			break;
		case GUI_AnchorType::BOTTOM_CENTER:
			m_anchorPosition = center - parentWindow.GetPointAtUV(Vec2(0.5f, 0.f));
			break;
		case GUI_AnchorType::BOTTOM_RIGHT:
			m_anchorPosition = center - parentWindow.GetPointAtUV(Vec2(1.f, 0.f));
			break;
		case GUI_AnchorType::HORIONTAL_STRETCH:
			offset = center - parentWindow.GetPointAtUV(Vec2(0.5f, 0.5f));
			m_anchorPosition = Vec2(0.f, offset.y);
			break;
		case GUI_AnchorType::VERTICAL_STRETCH:
			offset = center - parentWindow.GetPointAtUV(Vec2(0.5f, 0.5f));
			m_anchorPosition = Vec2(offset.x, 0.f);
			break;
		case GUI_AnchorType::FULL_STRETCH:
			m_anchorPosition = Vec2::ZERO;
			break;
		case GUI_AnchorType::NONE:
			m_originPosition = center;
			m_anchorPosition = Vec2::ZERO;
			break;
		default:
			break;
	}
	SetBufferDirty();
}


void GUI_Element::SetAnchorPosition(Vec2 const& position)
{
	m_anchorPosition = position;
	SetBufferDirty();
}



void GUI_Element::SetAnchorStretchOffset(Vec2 const& offset)
{
	m_anchorStretchOffset = offset;
	SetBufferDirty();
}


void GUI_Element::SetIsDraggable(bool isDraggable)
{
	m_isDraggable = isDraggable;
	//SetBufferDirty();
}


void GUI_Element::SetDraggableAreaUVs(AABB2 const& uvs)
{
	m_draggingAreaUVs = uvs;
	//SetBufferDirty();
}


void GUI_Element::SetParent(GUI_Element* parent)
{
	if (!parent)
	{
		SetBufferDirty();
	}
	m_parent = parent;
	SetBufferDirty();
}


void GUI_Element::SetChildren(std::vector<GUI_Element*> children)
{
	m_children = children;
	SetBufferDirty();
}


void GUI_Element::AddChild(GUI_Element* child)
{
	m_children.push_back(child);
	child->SetParent(this);
	SetBufferDirty();
}


void GUI_Element::SetBackgroundStyle(GUI_BackgroundStyle style)
{
	m_backgroundStyle = style;
	SetBufferDirty();
}


void GUI_Element::SetSliceDepth(Vec2 const& sliceDepth)
{
	m_sliceDepth = sliceDepth;
	SetBufferDirty();
}


void GUI_Element::SetBackgroundColor(Rgba8 const& color)
{
	m_backgroundColor = color;
	SetBufferDirty();
}


void GUI_Element::SetBackgroundUVs(AABB2 const& uvs)
{
	m_backgroundUVs = uvs;
	SetBufferDirty();
}


void GUI_Element::SetBackgroundTexture(Texture* texture)
{
	m_backgroundTexture = texture;
	SetBufferDirty();
}


void GUI_Element::SetBorderStyle(GUI_BorderStyle style)
{
	m_borderStyle = style;
	SetBufferDirty();
}


void GUI_Element::SetBorderColor(Rgba8 const& color)
{
	m_borderColor = color;
	SetBufferDirty();
}


void GUI_Element::SetBorderThickness(float thickness)
{
	m_borderThickness = thickness;
	SetBufferDirty();
}


void GUI_Element::SetRoundBorderRadius(float radius)
{
	m_roundBorderRadius = radius;
	SetBufferDirty();
}


void GUI_Element::SetBorderGap(Vec2 borderGap)
{
	m_borderGap = borderGap;
	SetBufferDirty();
}

void GUI_Element::SetConstantBuffer()
{
	m_borderConstant.BoxMins = m_elementWindow.m_mins;
	m_borderConstant.BoxMaxs = m_elementWindow.m_maxs;
	m_borderConstant.Radius = m_roundBorderRadius;
}


void GUI_Element::BindConstantBuffer(Renderer* renderer)
{
	if (m_borderCBO)
	{
		delete m_borderCBO;
		m_borderCBO = nullptr;
	}
	m_borderCBO = renderer->CreateConstantBuffer((unsigned int)sizeof(BorderConstant));
	renderer->CopyCPUToGPU(&m_borderConstant, sizeof(m_borderConstant), m_borderCBO);
	renderer->BindConstantBuffer(BORDER_CONSTANT_BUFFER_SLOT, m_borderCBO);
}


void GUI_Element::SetCanHover(bool canHover)
{
	m_canHover = canHover;
}


void GUI_Element::SetIsHover(bool isHover)
{
	m_isHover = isHover;
	SetBufferDirty();
}


void GUI_Element::SetHoverTexture(Texture* texture)
{
	m_hoverTexture = texture;
	SetBufferDirty();
}


void GUI_Element::SetHoverColor(Rgba8 const& color)
{
	m_hoverColor = color;
	SetBufferDirty();
}


void GUI_Element::SetIsFocus(bool isFocus)
{
	m_isFocus = isFocus;
	SetBufferDirty();
}

void GUI_Element::SetFocusTexture(Texture* texture)
{
	m_focusTexture = texture;
	SetBufferDirty();
}


void GUI_Element::SetFocusColor(Rgba8 const& color)
{
	m_focusColor = color;
	SetBufferDirty();
}


void GUI_Element::SetAnimationTimer(float duration)
{
	m_animationDuration = duration;
	SetBufferDirty();
}


void GUI_Element::SetAnimationStyle(GUI_AnimationStyle style)
{
	m_animationStyle = style;
	SetBufferDirty();
}


void GUI_Element::SetBufferDirty()
{
	m_isBufferDirty = true;
	for (GUI_Element* child : m_children)
	{
		if (child) child->SetBufferDirty();
	}
}


bool GUI_Element::IsPointInside(Vec2 const& mousePos) const
{
	switch (m_borderStyle)
	{
		case GUI_BorderStyle::NONE:
		case GUI_BorderStyle::RECT:
		case GUI_BorderStyle::DOUBLE_RECT:
		case GUI_BorderStyle::SINGLE_DIVISION:
		case GUI_BorderStyle::DOUBLE_DIVISION:
		case GUI_BorderStyle::DOUBLE_DIVISION_GAP:
			return m_elementWindow.IsPointInside(mousePos);
			break;
		case GUI_BorderStyle::ROUNDED_NONE:
		case GUI_BorderStyle::ROUNDED:
		case GUI_BorderStyle::DOUBLE_ROUNDED:
		{
			Vec2 bottomLeft = m_elementWindow.m_mins + Vec2(m_roundBorderRadius, m_roundBorderRadius);
			Vec2 bottomRight = Vec2(m_elementWindow.m_maxs.x, m_elementWindow.m_mins.y) + Vec2(-m_roundBorderRadius, m_roundBorderRadius);
			Vec2 topRight = m_elementWindow.m_maxs + Vec2(-m_roundBorderRadius, -m_roundBorderRadius);
			Vec2 topLeft = Vec2(m_elementWindow.m_mins.x, m_elementWindow.m_maxs.y) + Vec2(m_roundBorderRadius, -m_roundBorderRadius);
			float radiusSqaured = m_roundBorderRadius * m_roundBorderRadius;
			if (mousePos.x < bottomLeft.x && mousePos.y < bottomLeft.y)
			{
				if ((mousePos - bottomLeft).GetLengthSquared() > radiusSqaured) return false;
			}
			else if (mousePos.x > bottomRight.x && mousePos.y < bottomRight.y)
			{
				if ((mousePos - bottomRight).GetLengthSquared() > radiusSqaured) return false;
			}
			else if (mousePos.x > topRight.x && mousePos.y > topRight.y)
			{
				if ((mousePos - topRight).GetLengthSquared() > radiusSqaured) return false;
			}
			else if (mousePos.x < topLeft.x && mousePos.y > topLeft.y)
			{
				if ((mousePos - topLeft).GetLengthSquared() > radiusSqaured) return false;
			}
			
			return m_elementWindow.IsPointInside(mousePos);
			break;
		}
		default:
			return false;
	}
}


bool GUI_Element::IsElementDraggable(Vec2 const& mousePos) const
{
	if (m_isDraggable)
	{
		AABB2 draggableArea = m_elementWindow.GetBoxWithIn(m_draggingAreaUVs);
		return draggableArea.IsPointInside(mousePos);
	}

	return false;
}


void GUI_Element::ReconstructElement()
{
	CalculateElementWindow();
	CreateBorderVerts();
	CreateBackgroundVerts();
	CreateHoverAndFocusVerts();
}


void GUI_Element::CalculateElementWindow()
{
	AABB2 parentWindow;
	if (m_parent)
	{
		parentWindow = m_parent->m_elementWindow;
	}
	else
	{
		IntVec2 const& windowDimensions = Window::GetWindowContext()->GetClientDimensions();
		float windowSizeX = static_cast<float>(windowDimensions.x);
		float windowSizeY = static_cast<float>(windowDimensions.y);
		parentWindow = AABB2(Vec2::ZERO, Vec2(windowSizeX, windowSizeY));
	}
	Vec2 center, mins, maxs, size;

	switch (m_anchorType)
	{
		case GUI_AnchorType::TOP_LEFT:
			center = Vec2(parentWindow.m_mins.x, parentWindow.m_maxs.y) + m_anchorPosition;
			m_elementWindow = AABB2(center - 0.5f * m_size, center + 0.5f * m_size);
			break;
		case GUI_AnchorType::TOP_CENTER:
			center = Vec2(0.5f * (parentWindow.m_mins.x + parentWindow.m_maxs.x), parentWindow.m_maxs.y) + m_anchorPosition;
			m_elementWindow = AABB2(center - 0.5f * m_size, center + 0.5f * m_size);
			break;
		case GUI_AnchorType::TOP_RIGHT:
			center = Vec2(parentWindow.m_maxs.x, parentWindow.m_maxs.y) + m_anchorPosition;
			m_elementWindow = AABB2(center - 0.5f * m_size, center + 0.5f * m_size);
			break;
		case GUI_AnchorType::MIDDLE_LEFT:
			center = Vec2(parentWindow.m_mins.x, 0.5f * (parentWindow.m_mins.y + parentWindow.m_maxs.y)) + m_anchorPosition;
			m_elementWindow = AABB2(center - 0.5f * m_size, center + 0.5f * m_size);
			break;
		case GUI_AnchorType::CENTER:
			center = Vec2(0.5f * (parentWindow.m_mins.x + parentWindow.m_maxs.x), 0.5f * (parentWindow.m_mins.y + parentWindow.m_maxs.y)) + m_anchorPosition;
			m_elementWindow = AABB2(center - 0.5f * m_size, center + 0.5f * m_size);
			break;
		case GUI_AnchorType::MIDDLE_RIGHT:
			center = Vec2(parentWindow.m_maxs.x, 0.5f * (parentWindow.m_mins.y + parentWindow.m_maxs.y)) + m_anchorPosition;
			m_elementWindow = AABB2(center - 0.5f * m_size, center + 0.5f * m_size);
			break;
		case GUI_AnchorType::BOTTOM_LEFT:
			center = Vec2(parentWindow.m_mins.x, parentWindow.m_mins.y) + m_anchorPosition;
			m_elementWindow = AABB2(center - 0.5f * m_size, center + 0.5f * m_size);
			break;
		case GUI_AnchorType::BOTTOM_CENTER:
			center = Vec2(0.5f * (parentWindow.m_mins.x + parentWindow.m_maxs.x), parentWindow.m_mins.y) + m_anchorPosition;
			m_elementWindow = AABB2(center - 0.5f * m_size, center + 0.5f * m_size);
			break;
		case GUI_AnchorType::BOTTOM_RIGHT:
			center = Vec2(parentWindow.m_maxs.x, parentWindow.m_mins.y) + m_anchorPosition;
			m_elementWindow = AABB2(center - 0.5f * m_size, center + 0.5f * m_size);
			break;
		case GUI_AnchorType::HORIONTAL_STRETCH:
			center = Vec2(0.5f * (parentWindow.m_mins.x + parentWindow.m_maxs.x), 0.5f * (parentWindow.m_mins.y + parentWindow.m_maxs.y) + m_anchorPosition.y);
			size = Vec2(parentWindow.m_maxs.x - parentWindow.m_mins.x - 2.f * m_anchorStretchOffset.x, m_size.y);
			m_elementWindow = AABB2(center - 0.5f * size, center + 0.5f * size);
			break;
		case GUI_AnchorType::VERTICAL_STRETCH:
			center = Vec2(0.5f * (parentWindow.m_mins.x + parentWindow.m_maxs.x) + m_anchorPosition.x, 0.5f * (parentWindow.m_mins.y + parentWindow.m_maxs.y));
			size = Vec2(m_size.x, parentWindow.m_maxs.y - parentWindow.m_mins.y - 2.f * m_anchorStretchOffset.y);
			m_elementWindow = AABB2(center - 0.5f * size, center + 0.5f * size);
			break;
		case GUI_AnchorType::FULL_STRETCH:
			center = Vec2(0.5f * (parentWindow.m_mins.x + parentWindow.m_maxs.x) + m_anchorPosition.x, 0.5f * (parentWindow.m_mins.y + parentWindow.m_maxs.y) + m_anchorPosition.y);
			size = Vec2(parentWindow.m_maxs.x - parentWindow.m_mins.x - 2.f * m_anchorStretchOffset.x, parentWindow.m_maxs.y - parentWindow.m_mins.y - 2.f * m_anchorStretchOffset.y);
			m_elementWindow = AABB2(center - 0.5f * size, center + 0.5f * size);
			break;
		case GUI_AnchorType::NONE:
			center = m_originPosition + m_anchorPosition;
			m_elementWindow = AABB2(center - 0.5f * m_size, center + 0.5f * m_size);
			break;
		default:
			break;
	}
}


void GUI_Element::CreateBorderVerts()
{
	m_borderVerts.clear();

	float halfThickness = 0.5f * m_borderThickness;
	AABB2 borderBox(m_elementWindow.m_mins + Vec2(-halfThickness, -halfThickness), m_elementWindow.m_maxs + Vec2(halfThickness, halfThickness));
	switch (m_borderStyle)
	{
		case GUI_BorderStyle::NONE:
			break;
		case GUI_BorderStyle::ROUNDED_NONE:
			break;
		case GUI_BorderStyle::RECT:
			AddVertsForRectBorderUI(m_borderVerts, borderBox, m_borderThickness, m_borderColor);
			break;
		case GUI_BorderStyle::ROUNDED:
			AddVertsForRoundedBorderUI(m_borderVerts, borderBox, m_borderThickness, m_roundBorderRadius, m_borderColor);
			break;
		case GUI_BorderStyle::DOUBLE_RECT:
			AddVertsForDoubleRectBorderUI(m_borderVerts, borderBox, m_borderThickness, 2.f * m_borderThickness, m_borderColor);
			break;
		case GUI_BorderStyle::DOUBLE_ROUNDED:
			AddVertsForDoubleRoundedBorderUI(m_borderVerts, borderBox, m_borderThickness, m_roundBorderRadius, 2.f * m_borderThickness, m_borderColor);
			break;
		case GUI_BorderStyle::SINGLE_DIVISION:
		{
			AABB2 box(m_elementWindow.m_mins + Vec2(halfThickness, halfThickness), m_elementWindow.m_maxs + Vec2(-halfThickness, -halfThickness));
			LineSegment2 line(box.GetPointAtUV(Vec2(0.f, 0.5f)), box.GetPointAtUV(Vec2(1.f, 0.5f)));
			AddVertsForLineSegment2D(m_borderVerts, line, m_borderThickness, m_borderColor);
			break;
		}
		case GUI_BorderStyle::DOUBLE_DIVISION:
		{
			AABB2 box(m_elementWindow.m_mins + Vec2(halfThickness, halfThickness), m_elementWindow.m_maxs + Vec2(-halfThickness, -halfThickness));
			LineSegment2 topLine(box.GetPointAtUV(Vec2(0.f, 1.f)), box.GetPointAtUV(Vec2(1.f, 1.f)));
			LineSegment2 bottomLine(box.GetPointAtUV(Vec2(0.f, 0.f)), box.GetPointAtUV(Vec2(1.f, 0.f)));
			AddVertsForLineSegment2D(m_borderVerts, topLine, m_borderThickness, m_borderColor);
			AddVertsForLineSegment2D(m_borderVerts, bottomLine, m_borderThickness, m_borderColor);
			break;
		}
		case GUI_BorderStyle::DOUBLE_DIVISION_GAP:
		{
			AABB2 box(m_elementWindow.m_mins + Vec2(halfThickness, halfThickness), m_elementWindow.m_maxs + Vec2(-halfThickness, -halfThickness));
			LineSegment2 topLeftLine(box.GetPointAtUV(Vec2(0.f, 1.f)), box.GetPointAtUV(Vec2(m_borderGap.x, 1.f)));
			LineSegment2 topRightLine(box.GetPointAtUV(Vec2(m_borderGap.y, 1.f)), box.GetPointAtUV(Vec2(1.f, 1.f)));
			LineSegment2 bottomLeftLine(box.GetPointAtUV(Vec2(0.f, 0.f)), box.GetPointAtUV(Vec2(m_borderGap.x, 0.f)));
			LineSegment2 bottomRightLine(box.GetPointAtUV(Vec2(m_borderGap.y, 0.f)), box.GetPointAtUV(Vec2(1.f, 0.f)));
			AddVertsForLineSegment2D(m_borderVerts, topLeftLine, m_borderThickness, m_borderColor);
			AddVertsForLineSegment2D(m_borderVerts, topRightLine, m_borderThickness, m_borderColor);
			AddVertsForLineSegment2D(m_borderVerts, bottomLeftLine, m_borderThickness, m_borderColor);
			AddVertsForLineSegment2D(m_borderVerts, bottomRightLine, m_borderThickness, m_borderColor);
			Vec2 normal(1.f, 0.f);
			normal.RotateDegrees(45.f);
			Vec2 obbCenter = box.GetPointAtUV(Vec2(0.5f * (m_borderGap.x + m_borderGap.y), 0.5f));
			OBB2 obb(obbCenter, normal, Vec2(0.5f * box.GetDimensions().y, 0.5f * box.GetDimensions().y));
			AddVertsForOBB2D(m_borderVerts, obb, Rgba8(100, 100, 255, 255));
			break;
		}
		default:
			break;
	}
}


void GUI_Element::CreateBackgroundVerts()
{
	m_backgroundVerts.clear();

	AABB2 const& backgroundBox = m_elementWindow.GetBoxWithIn(m_backgroundUVs);

	switch (m_backgroundStyle)
	{
		case GUI_BackgroundStyle::DEFAULT:
		{
			AddVertsForAABB2D(m_backgroundVerts, backgroundBox, m_backgroundColor);
			break;
		}
		case GUI_BackgroundStyle::NINE_SLICE:
		{
			Vec2 textureDimensions = (Vec2)m_backgroundTexture->GetDimensions();
			Vec2 screenDimensions = backgroundBox.GetDimensions();
			if (textureDimensions.x >= screenDimensions.x || textureDimensions.y >= screenDimensions.y)
			{
				AddVertsForAABB2D(m_backgroundVerts, backgroundBox, m_backgroundColor);
			}
			else
			{
				float firstXCut = m_sliceDepth.x;
				float secondXCut = 1.f - m_sliceDepth.x;
				float firstYCut = m_sliceDepth.y;
				float secondYCut = 1.f - m_sliceDepth.y;
				AABB2 UVs[9] = {};
				UVs[BottomLeft] = AABB2(Vec2(0.f, 0.f), Vec2(firstXCut, firstYCut));
				UVs[BottomCenter] = AABB2(Vec2(firstXCut, 0.f), Vec2(secondXCut, firstYCut));
				UVs[BottomRight] = AABB2(Vec2(secondXCut, 0.f), Vec2(1.f, firstYCut));
				UVs[MiddleLeft] = AABB2(Vec2(0.f, firstYCut), Vec2(firstXCut, secondYCut));
				UVs[MiddleCenter] = AABB2(Vec2(firstXCut, firstYCut), Vec2(secondXCut, secondYCut));
				UVs[MiddleRight] = AABB2(Vec2(secondXCut, firstYCut), Vec2(1.f, secondYCut));
				UVs[TopLeft] = AABB2(Vec2(0.f, secondYCut), Vec2(firstXCut, 1.f));
				UVs[TopCenter] = AABB2(Vec2(firstXCut, secondYCut), Vec2(secondXCut, 1.f));
				UVs[TopRight] = AABB2(Vec2(secondXCut, secondYCut), Vec2(1.f, 1.f));

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
				Vec2 start = backgroundBox.m_mins;
				// bottom left
				AddVertsForAABB2D(m_backgroundVerts, AABB2(start, start + dimensionsPerCell), m_backgroundColor, UVs[BottomLeft].m_mins, UVs[BottomLeft].m_maxs);
				// bottom Right
				Vec2 bottomRightStart = start + Vec2(dimensionsPerCell.x + remainderWidth, 0.f);
				AddVertsForAABB2D(m_backgroundVerts, AABB2(bottomRightStart, bottomRightStart + dimensionsPerCell), m_backgroundColor, UVs[BottomRight].m_mins, UVs[BottomRight].m_maxs);
				// top left
				Vec2 topLeftStart = start + Vec2(0.f, dimensionsPerCell.y + remainderHeight);
				AddVertsForAABB2D(m_backgroundVerts, AABB2(topLeftStart, topLeftStart + dimensionsPerCell), m_backgroundColor, UVs[TopLeft].m_mins, UVs[TopLeft].m_maxs);
				// top right
				Vec2 topRightStart = topLeftStart + Vec2(dimensionsPerCell.x + remainderWidth, 0.f);
				AddVertsForAABB2D(m_backgroundVerts, AABB2(topRightStart, topRightStart + dimensionsPerCell), m_backgroundColor, UVs[TopRight].m_mins, UVs[TopRight].m_maxs);

				// bottom row
				Vec2 bottomStart = start + Vec2(dimensionsPerCell.x, 0.f);
				for (int index = 0; index < centerCountX; index++)
				{
					Vec2 mins = bottomStart + Vec2(widthPerCenterCell * static_cast<float>(index), 0.f);
					AddVertsForAABB2D(m_backgroundVerts, AABB2(mins, mins + edgeRowDimensions), m_backgroundColor, UVs[BottomCenter].m_mins, UVs[BottomCenter].m_maxs);
				}

				// top row
				Vec2 topStart = topLeftStart + Vec2(dimensionsPerCell.x, 0.f);
				for (int index = 0; index < centerCountX; index++)
				{
					Vec2 mins = topStart + Vec2(widthPerCenterCell * static_cast<float>(index), 0.f);
					AddVertsForAABB2D(m_backgroundVerts, AABB2(mins, mins + edgeRowDimensions), m_backgroundColor, UVs[TopCenter].m_mins, UVs[TopCenter].m_maxs);
				}

				// left column
				Vec2 leftStart = start + Vec2(0.f, dimensionsPerCell.y);
				for (int index = 0; index < centerCountY; index++)
				{
					Vec2 mins = leftStart + Vec2(0.f, heightPerCenterCell * static_cast<float>(index));
					AddVertsForAABB2D(m_backgroundVerts, AABB2(mins, mins + edgeColumnDimensions), m_backgroundColor, UVs[MiddleLeft].m_mins, UVs[MiddleLeft].m_maxs);
				}

				// right column
				Vec2 rightStart = bottomRightStart + Vec2(0.f, dimensionsPerCell.y);
				for (int index = 0; index < centerCountY; index++)
				{
					Vec2 mins = rightStart + Vec2(0.f, heightPerCenterCell * static_cast<float>(index));
					AddVertsForAABB2D(m_backgroundVerts, AABB2(mins, mins + edgeColumnDimensions), m_backgroundColor, UVs[MiddleRight].m_mins, UVs[MiddleRight].m_maxs);
				}

				// center
				Vec2 centerStart = start + dimensionsPerCell;
				for (int j = 0; j < centerCountY; j++)
				{
					for (int i = 0; i < centerCountX; i++)
					{
						Vec2 offset = Vec2(widthPerCenterCell * static_cast<float>(i), heightPerCenterCell * static_cast<float>(j));
						Vec2 mins = centerStart + offset;
						AddVertsForAABB2D(m_backgroundVerts, AABB2(mins, mins + centerDimensions), m_backgroundColor, UVs[MiddleCenter].m_mins, UVs[MiddleCenter].m_maxs);
					}
				}
			}
			break;
		}
		default:
			break;
	}
}


void GUI_Element::CreateHoverAndFocusVerts()
{
	m_hoverVerts.clear();
	m_focusVerts.clear();

	AABB2 const& box = m_elementWindow.GetBoxWithIn(m_backgroundUVs);
	AddVertsForAABB2D(m_hoverVerts, box, m_hoverColor);
	AddVertsForAABB2D(m_focusVerts, box, m_focusColor);
}


void GUI_Element::CreateAnimationVerts()
{
	m_animationVerts.clear();

	if (m_animationTimer == 0.f) return;

	AABB2 const& bounds = m_elementWindow;
	float timeFraction = m_animationTimer / m_animationDuration;
	Rgba8 bottomColor = InterpolateBetweenColor(Rgba8(255, 255, 255, 0), Rgba8(128, 128, 50, 200), timeFraction); 
	Rgba8 topColor = InterpolateBetweenColor(Rgba8(255, 255, 255, 0), Rgba8(255, 255, 255, 0), timeFraction);

	switch (m_animationStyle)
	{
		case GUI_AnimationStyle::NONE:
			break;
		case GUI_AnimationStyle::UP_GLOW:
			Vec3 pos0(Vec2(bounds.m_mins.x, bounds.m_mins.y));
			Vec3 pos1(Vec2(bounds.m_maxs.x, bounds.m_mins.y));
			Vec3 pos2(Vec2(bounds.m_maxs.x, bounds.m_maxs.y));
			Vec3 pos3(Vec2(bounds.m_mins.x, bounds.m_maxs.y));

			m_animationVerts.emplace_back(pos0, bottomColor, Vec2::ZERO);
			m_animationVerts.emplace_back(pos1, bottomColor, Vec2::ZERO);
			m_animationVerts.emplace_back(pos2, topColor, Vec2::ZERO);

			m_animationVerts.emplace_back(pos0, bottomColor, Vec2::ZERO);
			m_animationVerts.emplace_back(pos2, topColor, Vec2::ZERO);
			m_animationVerts.emplace_back(pos3, topColor, Vec2::ZERO);
			break;
	}
}


