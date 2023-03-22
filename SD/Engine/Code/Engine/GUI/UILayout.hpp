#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

#include <string>

class UIElement;

struct UILayout
{
public:
	void InitializeLayout(Renderer* renderer, BitmapFont* font);
	UIElement* CreateLayout(std::string const& layoutFile, AABB2 const& windowBound);

private:
	UIElement* CreateUIElementFromXmlElement(XmlElement const& element, AABB2 const& bounds);

public:
	Renderer* m_renderer = nullptr;
	BitmapFont* m_font = nullptr;
};


