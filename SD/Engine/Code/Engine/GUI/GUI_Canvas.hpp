#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"

#include <string>

class GUI_Element;
class Renderer;

class GUI_Canvas
{
public:
	GUI_Canvas(AABB2 const& canvasBounds = AABB2::ZERO_TO_ONE);
	~GUI_Canvas();

	void Update();
	void Render(Renderer* renderer) const;
	void RenderModel(Renderer* renderer) const;

	void LoadLayout(std::string const& layoutPath, Renderer* renderer);
	void ToggleOpen();
	bool IsOpen() const;
	bool IsTextfieldFocused() const;

	void SetRootElement(GUI_Element* root);
	void ResetHoverAndFocus();
	void UpdateHoverElement();
	void UpdateFocusElement();
	GUI_Element* FindElementByName(std::string const& name) const;
	GUI_Element* FindHoverElement(Vec2 const& mousePos) const;

public:
	AABB2 m_canvasBounds = AABB2::ZERO_TO_ONE;

	GUI_Element* m_rootElement = nullptr;
	GUI_Element* m_hoverElement = nullptr;
	GUI_Element* m_focusElement = nullptr;

	bool m_isOpen = false;

	bool m_isLeftMouseHeld = false;
	Vec2 m_dragOffset = Vec2::ZERO;
	bool m_isRightMouseHeld = false;
	Vec2 m_storedMousePosition = Vec2::ZERO;
};


