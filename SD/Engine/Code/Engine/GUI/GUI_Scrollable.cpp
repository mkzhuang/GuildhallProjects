#include "Engine/GUI/GUI_Scrollable.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/GUI/GUI_Canvas.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

GUI_Scrollable::GUI_Scrollable()
{
}


GUI_Scrollable::~GUI_Scrollable()
{
	m_children.clear();
	for (GUI_Element* button : m_entries)
	{
		delete button;
		button = nullptr;
	}
	m_entries.clear();
}


void GUI_Scrollable::Update()
{
	if (m_isHover)
	{
		if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
		{
			Vec2 mousePos = m_canvas->m_canvasBounds.GetPointAtUV(Window::GetWindowContext()->GetNormalizedCursorPos());
			AABB2 scrollbarBounds = m_elementWindow.GetBoxWithIn(m_scrollbarUVs);
			if (scrollbarBounds.IsPointInside(mousePos))
			{
				DebugAddMessage("inside scrollbar", 0.f, Rgba8::WHITE, Rgba8::WHITE);
				Vec2 scrollbarCenter = scrollbarBounds.GetCenter();
				if (mousePos.y < scrollbarCenter.y)
				{
					int maxTopIndex = (int)m_entries.size() - m_displaySize;
					if (m_currentTopIndex < maxTopIndex)
					{
						m_currentTopIndex++;
						SetBufferDirty();
					}
				}
				else if (mousePos.y > scrollbarCenter.y)
				{
					if (m_currentTopIndex > 0)
					{
						m_currentTopIndex--;
						SetBufferDirty();
					}
				}
			}
		}
		
		MouseWheelState state = g_theInput->GetMouseWheelState();
		switch (state)
		{
		case MouseWheelState::WHEEL_DOWN:
		{
			int maxTopIndex = (int)m_entries.size() - m_displaySize;
			if (m_currentTopIndex < maxTopIndex)
			{
				m_currentTopIndex++;
				SetBufferDirty();
			}
			break;
		}
		case MouseWheelState::WHEEL_UP:
		{
			if (m_currentTopIndex > 0)
			{
				m_currentTopIndex--;
				SetBufferDirty();
			}
			break;
		}
		case MouseWheelState::WHEEL_IDLE:
		default:
			break;
		}
	}

	if (m_isBufferDirty)
	{
		GUI_Element::ReconstructElement();
		ReconstructEntries();
		ReconstructScrollbar();
		m_isBufferDirty = false;
	}
}


void GUI_Scrollable::Render(Renderer* renderer) const
{
	GUI_Element::Render(renderer);

	if (m_hideScrollbarIfNotOverrun && (int)m_entries.size() - m_displaySize <= 0)
	{
		return;
	}

	if (m_buttonTexture)
	{
		renderer->BindTexture(m_buttonTexture);
		renderer->DrawVertexArray(m_buttonVerts);
	}
	else
	{
		renderer->BindTexture(m_trackTexture);
		renderer->DrawVertexArray(m_trackVerts);
	}

	renderer->BindTexture(m_thumbTexture);
	renderer->DrawVertexArray(m_thumbVerts);
}


void GUI_Scrollable::SetScrollbarRatio(float ratio)
{
	m_entrisUVs = AABB2(Vec2::ZERO, Vec2(ratio, 1.f));
	m_scrollbarUVs = AABB2(Vec2(ratio, 0.f), Vec2(1.f, 1.f));
	SetBufferDirty();
}


void GUI_Scrollable::ClearEntries()
{
	for (GUI_Element* element : m_entries)
	{
		element->m_parent = nullptr;
		delete element;
	}
	m_entries.clear();
	m_children.clear();

	SetBufferDirty();
}


void GUI_Scrollable::SetEntries(std::vector<GUI_Element*>& entries)
{
	ClearEntries();
	m_entries = std::move(entries);
	m_currentTopIndex = 0;
	SetBufferDirty();
}


void GUI_Scrollable::SetDisplaySize(int displaySize)
{
	m_displaySize = displaySize;
	SetBufferDirty();
}


void GUI_Scrollable::SetScrollbarVisibilityIfNotOverrun(bool hideIfNotOverrun)
{
	m_hideScrollbarIfNotOverrun = hideIfNotOverrun;
	SetBufferDirty();
}


void GUI_Scrollable::SetScrollbarTrackTexture(Texture* trackTexture)
{
	m_trackTexture = trackTexture;
	SetBufferDirty();
}


void GUI_Scrollable::SetScrollbarTrackColor(Rgba8 const& color)
{
	m_trackColor = color;
	SetBufferDirty();
}


void GUI_Scrollable::SetScrollbarThumbTexture(Texture* thumbTexture)
{
	m_thumbTexture = thumbTexture;
	SetBufferDirty();
}


void GUI_Scrollable::SetScrollbarThumbColor(Rgba8 const& color)
{
	m_thumbColor = color;
	SetBufferDirty();
}


void GUI_Scrollable::SetScrollbarButtonTexture(Texture* buttonTexture)
{
	m_buttonTexture = buttonTexture;
	SetBufferDirty();
}


void GUI_Scrollable::SetScrollbarButtonColor(Rgba8 const& color)
{
	m_buttonColor = color;
	SetBufferDirty();
}


void GUI_Scrollable::AddEntry(GUI_Element* entry)
{
	m_entries.push_back(entry);
	m_currentTopIndex = (int)m_entries.size() - m_displaySize;
	if (m_currentTopIndex < 0) m_currentTopIndex = 0;
	SetBufferDirty();
}


int const GUI_Scrollable::GetCurrentTopIndex() const
{
	return m_currentTopIndex;
}


void GUI_Scrollable::ReconstructEntries()
{
	for (GUI_Element* element : m_entries)
	{
		element->m_parent = nullptr;
	}
	m_children.clear();

	float entryHeightPercentage = 1.f / (float)m_displaySize;
	int maxIndex = m_currentTopIndex + m_displaySize;
	maxIndex = (int)m_entries.size() <= maxIndex ? (int)m_entries.size() : maxIndex;
	AABB2 entriesBox = m_elementWindow.GetBoxWithIn(m_entrisUVs);
	for (int index = m_currentTopIndex; index < maxIndex; index++)
	{
		GUI_Element*& element = m_entries[index];
		float heightOffset = (float)(index - m_currentTopIndex) * entryHeightPercentage;
		AABB2 entryBoxUVs(Vec2(0.01f, 1.f - entryHeightPercentage - heightOffset), Vec2(0.99f, 1.f - heightOffset));
		AABB2 const& entryBox = entriesBox.GetBoxWithIn(entryBoxUVs);
		element->SetBounds(entryBox);
		element->SetSize(entryBox.GetDimensions());
		AddChild(element);
	}
}


void GUI_Scrollable::ReconstructScrollbar()
{
	m_trackVerts.clear();
	AABB2 const& trackBox = m_elementWindow.GetBoxWithIn(m_scrollbarUVs);
	AddVertsForAABB2D(m_trackVerts, trackBox, m_trackColor);

	if (m_buttonTexture)
	{
		m_buttonVerts.clear();
		AABB2 const& topBoxUVs = m_scrollbarUVs.GetBoxWithIn(AABB2(Vec2(0.f, 0.975f), Vec2(1.f, 1.f)));
		AABB2 const& bottomBoxUVs = m_scrollbarUVs.GetBoxWithIn(AABB2(Vec2(0.f, 0.f), Vec2(1.f, 0.025f)));
		AABB2 const& topBox = m_elementWindow.GetBoxWithIn(topBoxUVs);
		AABB2 const& bottomBox = m_elementWindow.GetBoxWithIn(bottomBoxUVs);
		AddVertsForAABB2D(m_buttonVerts, topBox, m_buttonColor, AABB2::ZERO_TO_ONE);
		AddVertsForAABB2D(m_buttonVerts, bottomBox, m_buttonColor, AABB2::ONE_TO_ZERO);
	}

	m_thumbVerts.clear();
	float displayablePercent = (float)m_displaySize / (float)m_entries.size();
	float percentageAtTop = 1.f - (float)m_currentTopIndex / (float)m_entries.size();
	float percentageAtBottom = Clamp(percentageAtTop - displayablePercent, 0.f, 1.f);
	AABB2 const& thumbBoundUVs = m_scrollbarUVs.GetBoxWithIn(AABB2(Vec2(0.f, 0.025f), Vec2(1.f, 0.975f)));
	AABB2 const& thumbBounds = m_elementWindow.GetBoxWithIn(thumbBoundUVs);
	AABB2 thumbUVs(Vec2(0.f, percentageAtBottom), Vec2(1.f, percentageAtTop));
	AABB2 const& thumbBox = thumbBounds.GetBoxWithIn(thumbUVs);
	AddVertsForAABB2D(m_thumbVerts, thumbBox, m_thumbColor);
}


