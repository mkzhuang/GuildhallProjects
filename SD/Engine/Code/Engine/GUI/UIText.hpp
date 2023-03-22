#pragma once
#include "Engine/GUI/UIElement.hpp"

class BitmapFont;

class UIText : public UIElement
{
public:
	UIText();
	UIText(std::string const& name, AABB2 const& bound, UIAnchor const& anchor, Renderer* renderer, std::string const& text, BitmapFont * font, float textSize = 32.f, Vec2 const& alignment = Vec2(0.5f, 0.5f), bool isDraggable = false);
	~UIText();

public:
	void Update() override;
	void Render() const override;
	void OnClick() override;

	void SetText(std::string const& text);
	void SetTextSize(float textSize);
	void SetAlignment(Vec2 const& alignment);

public:
	std::string m_text = "";
	BitmapFont* m_font = nullptr;
	float m_textSize = 1.f;
	Vec2 m_alignment = Vec2::ZERO;
};


