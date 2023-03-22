#pragma once
#include "Engine/GUI/UIElement.hpp"
#include "Engine/Core/EngineCommon.hpp"

class UIButton : public UIElement
{
public:
	UIButton();
	UIButton(std::string const& name, AABB2 const& bound, UIAnchor const& anchor, Renderer* renderer, bool isDraggable = false);
	~UIButton();

	void Update() override;
	void Render() const override;
	void OnClick() override;

	void SetHandler(std::string const& handlerName);

public:
	std::string m_handlerName;
};