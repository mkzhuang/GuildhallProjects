#pragma once
#include "Engine/GUI/UIElement.hpp"

class UIScrollable : public UIElement
{
public:
	UIScrollable();
	UIScrollable(std::string const& name, AABB2 const& bound, UIAnchor const& anchor, Renderer* renderer, bool isDraggable, size_t totalEntries = 0, size_t displayableEntries = 0);
	~UIScrollable();

	void Update() override;
	void Render() const override;
	void OnClick() override;

	void SetTotalEntries(size_t totalEntries);
	void SetDisplayableEntries(size_t displayableEntries);
	bool Scroll(bool down);

	size_t const GetCurrentTopIndex();

public:
	size_t m_totalEntries = 0;
	size_t m_displayableEntries = 0;
	size_t m_currentTopIndex = 0;
};