#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/GUI/GUI_Element.hpp"

#include <string>

class GUI_Canvas;
class Renderer;

class GUI_Layout
{
public:
	GUI_Layout(Renderer* renderer);
	~GUI_Layout() {}
	GUI_Element* GenerateFromLayout(std::string const& layoutPath, GUI_Canvas* canvas);

private:
	GUI_Element* GenerateGUIElementFromXmlElement(XmlElement const& element, GUI_Canvas* canvas, GUI_Element*& parent);
	GUI_AnchorType GetAnchorTypeFromString(std::string const& anchorType);
	GUI_BackgroundStyle GetBackgroundStyleFromString(std::string const& backgroundStyle);
	GUI_BorderStyle GetBorderStyleFromString(std::string const& borderStyle);
	GUI_AnimationStyle GetAnimationStyleFromString(std::string const& animationStyle);

public:
	Renderer* m_renderer = nullptr;
};