#pragma once
#include "Engine/GUI/GUI_Element.hpp"
#include "Engine/GUI/GUI_Button.hpp"

class GUI_Scrollable : public GUI_Element
{
public:
	GUI_Scrollable();
	~GUI_Scrollable();

	void Update();
	void Render(Renderer* renderer) const;

	void SetScrollbarRatio(float ratio);
	void ClearEntries();
	void SetEntries(std::vector<GUI_Element*>& entries);
	void SetDisplaySize(int displaySize);

	void SetScrollbarVisibilityIfNotOverrun(bool hideIfNotOverrun);
	void SetScrollbarTrackTexture(Texture* trackTexture);
	void SetScrollbarTrackColor(Rgba8 const& color);
	void SetScrollbarThumbTexture(Texture* thumbTexture);
	void SetScrollbarThumbColor(Rgba8 const& color);
	void SetScrollbarButtonTexture(Texture* buttonTexture);
	void SetScrollbarButtonColor(Rgba8 const& color);

	void AddEntry(GUI_Element* entry);
	int const GetCurrentTopIndex() const;

private:
	void ReconstructEntries();
	void ReconstructScrollbar();

public:
	std::vector<GUI_Element*> m_entries;
	int m_displaySize = 0;
	int m_currentTopIndex = 0;
	AABB2 m_entrisUVs = AABB2(Vec2::ZERO, Vec2(0.95f, 1.f));
	AABB2 m_scrollbarUVs = AABB2(Vec2(0.95f, 0.f), Vec2(1.f, 1.f));

	bool m_hideScrollbarIfNotOverrun = false;
	Texture* m_trackTexture = nullptr;
	Rgba8 m_trackColor = Rgba8::WHITE;
	Texture* m_thumbTexture = nullptr;
	Rgba8 m_thumbColor = Rgba8::WHITE;
	Texture* m_buttonTexture = nullptr;
	Rgba8 m_buttonColor = Rgba8::WHITE;

	std::vector<Vertex_PCU> m_trackVerts;
	std::vector<Vertex_PCU> m_thumbVerts;
	std::vector<Vertex_PCU> m_buttonVerts;
};