#pragma once
#include "Engine/GUI/GUI_Element.hpp"

class BitmapFont;
class Shader;

class GUI_Text : public GUI_Element
{
public:
	GUI_Text();
	~GUI_Text();

	void Update();
	void Render(Renderer* renderer) const;

	void SetText(std::string const& text);
	void SetFont(BitmapFont* font);
	void SetShader(Shader* shader);
	void SetHasDropShadow(bool hasDropShadow);
	void SetTextColor(Rgba8 const& color);
	void SetShadowColor(Rgba8 const& color);
	void SetTextHeight(float textHeight);
	void SetTextAspect(float textAspect);
	void SetTextAlignment(Vec2 const& textAlignment);
	void SetIsParentRelative(bool isParentRelative, AABB2 const& boundsUVs);

	std::string const& GetText() const;

private:
	void CreateTextVerts();

public:
	std::string m_text = "";
	BitmapFont* m_font = nullptr;
	Shader* m_shader = nullptr;
	bool m_hasDropShadow = false;
	Rgba8 m_textColor = Rgba8::WHITE;
	Rgba8 m_textShadowColor = Rgba8::BLACK;
	float m_textHeight = 1.f;
	float m_textAspect = 1.f;
	Vec2 m_textAlignment = Vec2::ZERO;
	bool m_isParentWindowRelative = false;
	AABB2 m_relativeBoundUVs = AABB2::ZERO_TO_ONE;
	std::vector<Vertex_PCU> m_textVerts;
	std::vector<Vertex_PCU> m_shadowVerts;
};