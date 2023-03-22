#include "Engine/GUI/UICanvas.hpp"
#include "Engine/GUI/UIElement.hpp"
#include "Engine/GUI/UI3DViewer.hpp"
#include "Engine/GUI/UIText.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

UICanvas::UICanvas()
{
	IntVec2 const& windowDimensions = Window::GetWindowContext()->GetClientDimensions();
	m_screenWindow = AABB2(Vec2::ZERO, Vec2(windowDimensions));
}


UICanvas::UICanvas(AABB2 const& bound, UIAnchor const& anchor)
{
	AABB2 window = bound.GetBoxWithIn(anchor.m_fraction);
	window.Translate(anchor.m_offset);
	m_screenWindow = window;
}


UICanvas::~UICanvas()
{
	delete m_rootElement;
	for (std::string handler : m_handlerNames)
	{
		UnregisterEventHandler(handler);
	}
}


void UICanvas::Update()
{
	UpdateMouseLocation();

	m_rootElement->Update();

	UpdateOnHover();

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		m_wasMouseClicked = true;
		m_mouseClickLocation = m_currentMouseLocation;
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE) && m_wasMouseClicked)
	{
		Vec2 offset = m_currentMouseLocation - m_prevMouseLocation;
		offset.y = -offset.y;
		if (m_hoverElement)
		{
			if (m_hoverElement->m_isDraggable)
			{
				m_hoverElement->MoveWindow(offset);
			}
			else
			{
				UIElement* element = m_hoverElement;
				while (element->m_parent)
				{
					if (element->m_parent->m_isDraggable)
					{
						element->m_parent->MoveWindow(offset);
					}
					element = element->m_parent;
				}
			}
		}
	}
	else
	{
		m_wasMouseClicked = false;
	}
}


void UICanvas::Render(Renderer* renderer) const
{
	UNUSED(renderer)

	m_rootElement->Render();

	//std::vector<Vertex_PCU> verts;
	//if (m_focusElement)
	//{
	//	AddVertsForAABB2D(verts, m_focusElement->GetScreenWindow(), m_focusElement->GetFocusColor());
	//}
	//if (m_hoverElement && m_focusElement != m_hoverElement)
	//{
	//	AddVertsForAABB2D(verts, m_hoverElement->GetScreenWindow(), m_hoverElement->GetHoverColor());
	//}
	//renderer->BindTexture(nullptr);
	//renderer->DrawVertexArray(verts);
}


void UICanvas::RenderModel(Renderer* renderer) const
{
	UNUSED(renderer)

	if (m_viewerElement) dynamic_cast<UI3DViewer*>(m_viewerElement)->RenderModel();
}


void UICanvas::UpdateMouseLocation()
{
	m_prevMouseLocation = m_currentMouseLocation;
	m_currentMouseLocation = g_theInput->GetMouseClientPosition();
}


void UICanvas::UpdateOnHover()
{
	Vec2 mouseUV = Window::GetWindowContext()->GetNormalizedCursorPos();
	Vec2 mousePos = mouseUV * Vec2(Window::GetWindowContext()->GetClientDimensions());
	UIElement* hoverElement = FindHoverElement(m_rootElement, mousePos);
	if (hoverElement == m_hoverElement) return;
	
	// #ToDo: inherited hover and focus if necessary in the future
	//UIElement* currentElement = m_hoverElement;
	//while (currentElement)
	//{
	//	currentElement->SetHover(false);
	//	currentElement = currentElement->m_parent;
	//}

	//currentElement = hoverElement;
	//while (currentElement)
	//{
	//	currentElement->SetHover(true);
	//	currentElement = currentElement->m_parent;
	//}

	if (m_hoverElement) m_hoverElement->SetHover(false);
	if (hoverElement)
	{
		m_hoverElement = hoverElement;
	}
	else
	{
		m_hoverElement = nullptr;
	}
	if (m_hoverElement) m_hoverElement->SetHover(true);
}


void UICanvas::UpdateOnFocus()
{
	if (m_hoverElement) 
	{
		m_hoverElement->OnClick();
		if (m_focusElement) m_focusElement->SetFocus(false);
		m_focusElement = m_hoverElement;
		if (m_focusElement) m_focusElement->SetFocus(true);
	}
}


void UICanvas::ClearFocusAndHover()
{
	if (m_hoverElement) 
	{
		m_hoverElement->SetHover(false);
		m_hoverElement = nullptr;
	}

	if (m_focusElement)
	{
		m_focusElement->SetFocus(false);
		m_focusElement = nullptr;
	}
}


UIElement* UICanvas::FindLowestCommonAncestor(UIElement* elementA, UIElement* elementB)
{
	UIElement* searchElement = elementA;
	while(searchElement)
	{ 
		if (searchElement == elementB)
		{
			return elementB;
		}
		else
		{
			searchElement = elementA->m_parent;
		}
	}

	searchElement = elementB;
	while (searchElement)
	{
		if (searchElement == elementA)
		{
			return elementA;
		}
		else
		{
			searchElement = elementB->m_parent;
		}
	}

	return nullptr;
	// to do, if one is not parent of the other

}


UIElement* UICanvas::FindElementByName(UIElement* currentNode, std::string const& name)
{
	if (currentNode->GetName() == name) return currentNode;

	if (currentNode->m_children.size() != 0)
	{
		UIElement* result = nullptr;
		for (UIElement* childElement : currentNode->m_children)
		{
			result = FindElementByName(childElement, name);
			if (result != nullptr)
			{
				return result;
			}
		}
		//return currentNode;
	}

	return nullptr;
}


UIElement* UICanvas::FindViewerElement(UIElement* currentNode)
{
	if (currentNode->m_hasOwnCamera) return currentNode;

	if (currentNode->m_children.size() != 0)
	{
		UIElement* result = nullptr;
		for (UIElement* childElement : currentNode->m_children)
		{
			result = FindViewerElement(childElement);
			if (result != nullptr)
			{
				return result;
			}
		}
		//return currentNode;
	}

	return nullptr;
}


UIElement* UICanvas::FindHoverElement(UIElement* currentNode, Vec2 const& mousePos)
{
	if (currentNode->GetScreenWindow().IsPointInside(mousePos))
	{
		if (currentNode->m_children.size() != 0)
		{
			UIElement* result = nullptr;
			for (UIElement* childElement : currentNode->m_children)
			{
				result = FindHoverElement(childElement, mousePos);
				if (result != nullptr)
				{
					return result;
				}
			}
			return currentNode;
		}
		else
		{
			return currentNode;
		}
	}

	return nullptr;
} 


void UICanvas::SetRootElement(UIElement* uiRoot)
{
	m_rootElement = uiRoot;
}


void UICanvas::SetViewerElement()
{
	m_viewerElement = FindViewerElement(m_rootElement);
}


void UICanvas::AddHandler(std::string const& handlerName)
{
	m_handlerNames.push_back(handlerName);
}


void UICanvas::RemoveHandler(std::string const& handlerName)
{
	for (int index = 0; index < (int)m_handlerNames.size(); index++)
	{
		if (m_handlerNames[index] == handlerName)
		{
			m_handlerNames.erase(m_handlerNames.begin() + index);
			return;
		}
	}
	return;
}


