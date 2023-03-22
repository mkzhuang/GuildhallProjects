#include "Engine/GUI/GUI_Canvas.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/GUI/GUI_Layout.hpp"
#include "Engine/GUI/GUI_Element.hpp"
#include "Engine/GUI/GUI_Textfield.hpp"
#include "Engine/GUI/GUI_ModelViewer.hpp"
#include "Engine/GUI/GUI_Button.hpp"

#include <deque>

bool gui_isEditingTextfield = false;

GUI_Canvas::GUI_Canvas(AABB2 const& canvasBounds)
	: m_canvasBounds(canvasBounds)
{
}


GUI_Canvas::~GUI_Canvas()
{
	delete m_rootElement;
}


void GUI_Canvas::Update()
{
	if (!m_isOpen) return;

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE) && !m_isLeftMouseHeld)
	{
		UpdateFocusElement();
		if (m_hoverElement)
		{
			m_isLeftMouseHeld = true;
			Vec2 mousePos = m_canvasBounds.GetPointAtUV(Window::GetWindowContext()->GetNormalizedCursorPos());
			m_dragOffset = m_hoverElement->m_anchorPosition - mousePos;
		}
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE) && m_isLeftMouseHeld)
	{
		Vec2 mousePos = m_canvasBounds.GetPointAtUV(Window::GetWindowContext()->GetNormalizedCursorPos());
		if (m_hoverElement->IsElementDraggable(mousePos))
		{
			Vec2 offset = mousePos + m_dragOffset - m_hoverElement->m_anchorPosition;
			m_hoverElement->SetAnchorPosition(m_hoverElement->m_anchorPosition + offset);
		}
	}

	if (!g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		m_isLeftMouseHeld = false;
		m_dragOffset = Vec2::ZERO;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE) && !m_isRightMouseHeld)
	{
		m_isRightMouseHeld = true;
		m_storedMousePosition = m_canvasBounds.GetPointAtUV(Window::GetWindowContext()->GetNormalizedCursorPos());
	}

	if (!g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE))
	{
		m_isRightMouseHeld = false;
	}

	std::deque<GUI_Element*> elementsQueue;
	elementsQueue.push_back(m_rootElement);
	while (!elementsQueue.empty())
	{
		GUI_Element* currentElement = elementsQueue.front();
		elementsQueue.pop_front();

		currentElement->Update();

		for (GUI_Element* childElement : currentElement->m_children)
		{
			elementsQueue.push_back(childElement);
		}
	}

	UpdateHoverElement();
}


void GUI_Canvas::Render(Renderer* renderer) const
{
	if (!m_isOpen) return;

	std::deque<GUI_Element*> elementsQueue;
	elementsQueue.push_back(m_rootElement);
	while (!elementsQueue.empty())
	{
		GUI_Element const* currentElement = elementsQueue.front();
		elementsQueue.pop_front();

		for (GUI_Element* childElement : currentElement->m_children)
		{
			elementsQueue.push_back(childElement);
		}

		currentElement->Render(renderer);
	}
}


void GUI_Canvas::RenderModel(Renderer* renderer) const
{
	if (!m_isOpen) return;

	std::deque<GUI_Element*> elementsQueue;
	elementsQueue.push_back(m_rootElement);
	while (!elementsQueue.empty())
	{
		GUI_Element const* currentElement = elementsQueue.front();
		elementsQueue.pop_front();

		for (GUI_Element* childElement : currentElement->m_children)
		{
			elementsQueue.push_back(childElement);
		}

		GUI_ModelViewer const* modelViewer = dynamic_cast<GUI_ModelViewer const*>(currentElement);
		if (modelViewer) modelViewer->RenderModel(renderer);
	}
}


void GUI_Canvas::LoadLayout(std::string const& layoutPath, Renderer* renderer)
{
	GUI_Layout layout(renderer);
	GUI_Element* rootElement = layout.GenerateFromLayout(layoutPath, this);
	SetRootElement(rootElement);
}


void GUI_Canvas::ToggleOpen()
{
	if (IsTextfieldFocused()) return;
	m_isOpen = !m_isOpen;
	if (!m_isOpen)
	{
		m_focusElement = nullptr;
		m_hoverElement = nullptr;
	}
}


bool GUI_Canvas::IsOpen() const
{
	return m_isOpen;
}


bool GUI_Canvas::IsTextfieldFocused() const
{
	if (!m_isOpen) return false;
	if (m_focusElement)
	{
		GUI_Textfield* textfield = dynamic_cast<GUI_Textfield*>(m_focusElement);
		return textfield;
	}
	return false;
}


void GUI_Canvas::SetRootElement(GUI_Element* root)
{
	m_rootElement = root;
}


void GUI_Canvas::ResetHoverAndFocus()
{
	m_hoverElement = nullptr;
	m_focusElement = nullptr;
}


void GUI_Canvas::UpdateHoverElement()
{
	if (m_isLeftMouseHeld) return;
	Vec2 mousePos = m_canvasBounds.GetPointAtUV(Window::GetWindowContext()->GetNormalizedCursorPos());
	GUI_Element* element = FindHoverElement(mousePos);
	if (m_hoverElement == element) return;
	if (m_hoverElement)
	{
		m_hoverElement->SetIsHover(false);
		GUI_Element* hover = m_hoverElement->m_parent;
		while (hover)
		{
			hover->SetIsHover(false);
			hover = hover->m_parent;
		}
	}
	m_hoverElement = element;
	if (m_hoverElement)
	{
		m_hoverElement->SetIsHover(true);
		GUI_Element* hover = m_hoverElement->m_parent;
		while (hover)
		{
			hover->SetIsHover(true);
			hover = hover->m_parent;
		}
	}
}


void GUI_Canvas::UpdateFocusElement()
{
	if (m_hoverElement)
	{
		if (m_focusElement == m_hoverElement)
		{
			GUI_Button* button = dynamic_cast<GUI_Button*>(m_focusElement);
			if (button) button->OnClick();
			return;
		}
		if (m_focusElement)
		{
			m_focusElement->SetIsFocus(false);
		}
		m_focusElement = m_hoverElement;
		if (m_focusElement)
		{
			if (dynamic_cast<GUI_Textfield*>(m_focusElement))
			{
				gui_isEditingTextfield = true;
			}
			m_focusElement->SetIsFocus(true);
		}
	}
	else
	{
		if (m_focusElement)
		{
			m_focusElement->SetIsFocus(false);
			gui_isEditingTextfield = false;
		}
		m_focusElement = nullptr;
	}

	if (m_focusElement)
	{
		GUI_Button* button = dynamic_cast<GUI_Button*>(m_focusElement);
		if (button) button->OnClick();
	}
}


GUI_Element* GUI_Canvas::FindElementByName(std::string const& name) const
{
	std::deque<GUI_Element*> elementsQueue;
	elementsQueue.push_back(m_rootElement);
	while (!elementsQueue.empty())
	{
		GUI_Element* currentElement = elementsQueue.front();
		elementsQueue.pop_front();

		if (currentElement->m_name == name)
		{
			return currentElement;
		}
		else
		{
			for (GUI_Element* childElement : currentElement->m_children)
			{
				elementsQueue.push_back(childElement);
			}
		}
	}

	return nullptr;
}


GUI_Element* GUI_Canvas::FindHoverElement(Vec2 const& mousePos) const
{
	GUI_Element* hoverElement = nullptr;
	std::deque<GUI_Element*> elementsQueue;
	elementsQueue.push_back(m_rootElement);
	while (!elementsQueue.empty())
	{
		GUI_Element* currentElement = elementsQueue.front();
		elementsQueue.pop_front();

		if (currentElement->IsPointInside(mousePos) && currentElement->m_canHover)
		{
			hoverElement = currentElement;
		}

		for (GUI_Element* childElement : currentElement->m_children)
		{
			elementsQueue.push_back(childElement);
		}
	}

	return hoverElement;
}


