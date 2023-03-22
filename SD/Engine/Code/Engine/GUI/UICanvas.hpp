#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/EngineCommon.hpp"

class UIElement;
class Renderer;

struct UIAnchor
{
public:
	UIAnchor() {}
	UIAnchor(AABB2 fraction, Vec2 offset = Vec2::ZERO)
		: m_fraction(fraction)
		, m_offset(offset)
	{
	}

public:
	AABB2 m_fraction = AABB2::ZERO_TO_ONE;
	Vec2 m_offset = Vec2::ZERO;
};

class UICanvas
{
public:
	UICanvas();
	UICanvas(AABB2 const& bound, UIAnchor const& anchor);
	~UICanvas();

	void Update();
	void Render(Renderer* renderer) const;
	void RenderModel(Renderer* renderer) const;
	void UpdateMouseLocation();
	void UpdateOnHover();
	void UpdateOnFocus();
	void ClearFocusAndHover();
	UIElement* FindLowestCommonAncestor(UIElement* elementA, UIElement* elementB);
	UIElement* FindElementByName(UIElement* currentNode, std::string const& name);
	UIElement* FindViewerElement(UIElement* currentNode);
	UIElement* FindHoverElement(UIElement* currentNode, Vec2 const& mousePos);
	void SetRootElement(UIElement* uiRoot);
	void SetViewerElement();

	void AddHandler(std::string const& handlerName);
	void RemoveHandler(std::string const& handlerName);


public:
	UIAnchor m_anchor;
	AABB2 m_screenWindow = AABB2::ZERO_TO_ONE;
	UIElement* m_hoverElement = nullptr;
	UIElement* m_focusElement = nullptr;
	UIElement* m_rootElement = nullptr;
	UIElement* m_viewerElement = nullptr;
	bool m_isOpen = false;
	Vec2 m_prevMouseLocation = Vec2::ZERO;
	Vec2 m_currentMouseLocation = Vec2::ZERO;
	Vec2 m_mouseClickLocation = Vec2::ZERO;
	bool m_wasMouseClicked = false;

	std::vector<std::string> m_handlerNames;
};