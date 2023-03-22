#include "Engine/GUI/GUI_Layout.hpp"
#include "Engine/GUI/GUI_Text.hpp"
#include "Engine/GUI/GUI_Textfield.hpp"
#include "Engine/GUI/GUI_Button.hpp"
#include "Engine/GUI/GUI_ModelViewer.hpp"
#include "Engine/GUI/GUI_Scrollable.hpp"
#include "Engine/GUI/GUI_Canvas.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "GUI_Scrollable.hpp"

GUI_Layout::GUI_Layout(Renderer* renderer)
	: m_renderer(renderer)
{
}

GUI_Element* GUI_Layout::GenerateFromLayout(std::string const& layoutPath, GUI_Canvas* canvas)
{
	XmlDocument doc;
	doc.LoadFile(layoutPath.c_str());
	XmlElement const* xmlRoot = doc.RootElement();

	GUI_Element* guiRoot = nullptr;
	if (xmlRoot)
	{
		guiRoot = GenerateGUIElementFromXmlElement(*xmlRoot, canvas, guiRoot);
	}
	return guiRoot;
}


GUI_Element* GUI_Layout::GenerateGUIElementFromXmlElement(XmlElement const& xmlElement, GUI_Canvas* canvas, GUI_Element*& parent)
{
	std::string const& type = ParseXmlAttribute(xmlElement, "type", "element");
	std::string const& name = ParseXmlAttribute(xmlElement, "name", "default");
	AABB2 const& boundUVs = ParseXmlAttribute(xmlElement, "boundUVs", AABB2::ZERO_TO_ONE);
	std::string const& anchorType = ParseXmlAttribute(xmlElement, "anchorType", "none");
	Vec2 const& anchorPosition = ParseXmlAttribute(xmlElement, "anchorPosition", Vec2::ZERO);
	Vec2 const& anchorOffset = ParseXmlAttribute(xmlElement, "anchorOffset", Vec2::ZERO);
	bool isDraggable = ParseXmlAttribute(xmlElement, "isDraggable", false);
	AABB2 const& draggableUVs = ParseXmlAttribute(xmlElement, "draggableUVs", AABB2::ZERO_TO_ONE);
	std::string const& backgroundType = ParseXmlAttribute(xmlElement, "backgroundType", "none");
	Vec2 const& backgroundSliceDepth = ParseXmlAttribute(xmlElement, "backgroundSliceDepth", Vec2::ZERO);
	Rgba8 const& backgroundColor = ParseXmlAttribute(xmlElement, "backgroundColor", Rgba8::CLEAR);
	AABB2 const& backgroundUVs = ParseXmlAttribute(xmlElement, "backgroundUVs", AABB2::ZERO_TO_ONE);
	std::string const& backgroundTexturePath = ParseXmlAttribute(xmlElement, "backgroundTexturePath", "");
	std::string const& borderType = ParseXmlAttribute(xmlElement, "borderType", "none");
	Rgba8 const& borderColor = ParseXmlAttribute(xmlElement, "borderColor", Rgba8::WHITE);
	float borderThickness = ParseXmlAttribute(xmlElement, "borderThickness", 0.1f);
	float borderRoundRadius = ParseXmlAttribute(xmlElement, "borderRoundRadius", 1.f);
	Vec2 const& borderGap = ParseXmlAttribute(xmlElement, "DoubleDivisionGap", Vec2::ZERO);
	std::string const& hoverTexturePath = ParseXmlAttribute(xmlElement, "hoverTexturePath", "");
	Rgba8 hoverColor = ParseXmlAttribute(xmlElement, "hoverColor", Rgba8::CLEAR);
	std::string const& focusTexturePath = ParseXmlAttribute(xmlElement, "focusTexturePath", "");
	Rgba8 focusColor = ParseXmlAttribute(xmlElement, "focusColor", Rgba8::CLEAR);
	bool canHover = ParseXmlAttribute(xmlElement, "canHover", true);
	std::string animationType = ParseXmlAttribute(xmlElement, "animationType", "none");
	float animationDuration = ParseXmlAttribute(xmlElement, "animationDuration", 0.f);

	GUI_Element* guiElement = nullptr;
	if (type == "text" || type == "textfield" || type == "button")
	{
		std::string const& text = ParseXmlAttribute(xmlElement, "text", "");
		std::string const& fontPath = ParseXmlAttribute(xmlElement, "fontPath", "Data/Fonts/SquirrelFixedFont");
		std::string const& fontMetaData = ParseXmlAttribute(xmlElement, "fontMetaData", "");
		std::string const& shaderPath = ParseXmlAttribute(xmlElement, "shaderPath", "");
		Rgba8 const& textColor = ParseXmlAttribute(xmlElement, "textColor", Rgba8::WHITE);
		bool hasDropShadow = ParseXmlAttribute(xmlElement, "hasDropShadow", false);
		Rgba8 const& shadowColor = ParseXmlAttribute(xmlElement, "shadowColor", Rgba8::BLACK);
		float textHeight = ParseXmlAttribute(xmlElement, "textHeight", 1.f);
		float textAspect = ParseXmlAttribute(xmlElement, "textAspect", 1.f);
		Vec2 const& textAlignment = ParseXmlAttribute(xmlElement, "textAlignment", Vec2::ZERO);

		BitmapFont* font = nullptr;
		if (fontMetaData.empty())
		{
			font = m_renderer->CreateOrGetBitmapFont(fontPath.c_str());
		}
		else
		{
			font = m_renderer->CreateOrGetBitmapFontWithMetadata(fontPath.c_str(), fontMetaData.c_str());
		}

		Shader* shader = nullptr;
		if (!shaderPath.empty())
		{
			shader = m_renderer->CreateOrGetShader(shaderPath.c_str());
		}

		if (type == "text")
		{
			guiElement = new GUI_Text();
		}
		else if (type == "textfield")
		{
			Rgba8 const& textBoxColor = ParseXmlAttribute(xmlElement, "textBoxColor", Rgba8::CLEAR);

			guiElement = new GUI_Textfield();

			static_cast<GUI_Textfield*>(guiElement)->SetTextBoxColor(textBoxColor);
		}
		else if (type == "button")
		{
			guiElement = new GUI_Button();
		}

		static_cast<GUI_Text*>(guiElement)->SetText(text);
		static_cast<GUI_Text*>(guiElement)->SetFont(font);
		static_cast<GUI_Text*>(guiElement)->SetShader(shader);
		static_cast<GUI_Text*>(guiElement)->SetTextColor(textColor);
		static_cast<GUI_Text*>(guiElement)->SetHasDropShadow(hasDropShadow);
		static_cast<GUI_Text*>(guiElement)->SetShadowColor(shadowColor);
		static_cast<GUI_Text*>(guiElement)->SetTextHeight(textHeight);
		static_cast<GUI_Text*>(guiElement)->SetTextAspect(textAspect);
		static_cast<GUI_Text*>(guiElement)->SetTextAlignment(textAlignment);
	}
	else if (type == "modelViewer")
	{
		guiElement = new GUI_ModelViewer();
	}
	else if (type == "scrollable")
	{
		float ratio = ParseXmlAttribute(xmlElement, "scrollbarRatio", 0.95f);
		int displaySize = ParseXmlAttribute(xmlElement, "displaySize", 10);
		bool hideScroolbarIfNotOverrun = ParseXmlAttribute(xmlElement, "hideScrollbarIfNotOverrun", false);
		std::string const& trackTexturePath = ParseXmlAttribute(xmlElement, "trackTexturePath", "");
		Rgba8 const& trackColor = ParseXmlAttribute(xmlElement, "trackColor", Rgba8::WHITE);
		std::string const& buttonTexturePath = ParseXmlAttribute(xmlElement, "buttonTexturePath", "");
		Rgba8 const& buttonColor = ParseXmlAttribute(xmlElement, "buttonColor", Rgba8::WHITE);
		std::string const& thumbTexturePath = ParseXmlAttribute(xmlElement, "thumbTexturePath", "");
		Rgba8 const& thumbColor = ParseXmlAttribute(xmlElement, "thumbColor", Rgba8::WHITE);

		Texture* trackTexture = trackTexturePath.empty() ? nullptr : m_renderer->CreateOrGetTextureFromFile(trackTexturePath.c_str());
		Texture* buttonTexture = buttonTexturePath.empty() ? nullptr : m_renderer->CreateOrGetTextureFromFile(buttonTexturePath.c_str());
		Texture* thumbTexture = thumbTexturePath.empty() ? nullptr : m_renderer->CreateOrGetTextureFromFile(thumbTexturePath.c_str());

		guiElement = new GUI_Scrollable();
		
		static_cast<GUI_Scrollable*>(guiElement)->SetScrollbarRatio(ratio);
		static_cast<GUI_Scrollable*>(guiElement)->SetDisplaySize(displaySize);
		static_cast<GUI_Scrollable*>(guiElement)->SetScrollbarVisibilityIfNotOverrun(hideScroolbarIfNotOverrun);
		static_cast<GUI_Scrollable*>(guiElement)->SetScrollbarTrackTexture(trackTexture);
		static_cast<GUI_Scrollable*>(guiElement)->SetScrollbarTrackColor(trackColor);
		static_cast<GUI_Scrollable*>(guiElement)->SetScrollbarButtonTexture(buttonTexture);
		static_cast<GUI_Scrollable*>(guiElement)->SetScrollbarButtonColor(buttonColor);
		static_cast<GUI_Scrollable*>(guiElement)->SetScrollbarThumbTexture(thumbTexture);
		static_cast<GUI_Scrollable*>(guiElement)->SetScrollbarThumbColor(thumbColor);
	}

	if (!guiElement) guiElement = new GUI_Element();
	AABB2 const& bounds = parent ? parent->m_elementWindow.GetBoxWithIn(boundUVs) : canvas->m_canvasBounds.GetBoxWithIn(boundUVs);
	Texture* backgroundTexture = backgroundTexturePath.empty() ? nullptr : m_renderer->CreateOrGetTextureFromFile(backgroundTexturePath.c_str());
	Texture* hoverTexture = hoverTexturePath.empty() ? backgroundTexture : m_renderer->CreateOrGetTextureFromFile(hoverTexturePath.c_str());
	Texture* focusTexture = focusTexturePath.empty() ? backgroundTexture : m_renderer->CreateOrGetTextureFromFile(focusTexturePath.c_str());
	hoverColor = hoverTexturePath.empty() ? backgroundColor : hoverColor;
	focusColor = focusTexturePath.empty() ? backgroundColor : focusColor;

	guiElement->SetCanvas(canvas);
	//guiElement->SetParent(parent);
	guiElement->SetName(name);
	guiElement->SetBounds(bounds);
	guiElement->SetSize(bounds.GetDimensions());
	guiElement->SetAnchorType(GetAnchorTypeFromString(anchorType));
	guiElement->SetAnchorPosition(anchorPosition);
	guiElement->SetAnchorStretchOffset(anchorOffset);
	guiElement->SetIsDraggable(isDraggable);
	guiElement->SetDraggableAreaUVs(draggableUVs);
	guiElement->SetBackgroundStyle(GetBackgroundStyleFromString(backgroundType));
	guiElement->SetSliceDepth(backgroundSliceDepth);
	guiElement->SetBackgroundColor(backgroundColor);
	guiElement->SetBackgroundUVs(backgroundUVs);
	guiElement->SetBackgroundTexture(backgroundTexture);
	guiElement->SetBorderStyle(GetBorderStyleFromString(borderType));
	guiElement->SetBorderColor(borderColor);
	guiElement->SetBorderThickness(borderThickness);
	guiElement->SetRoundBorderRadius(borderRoundRadius);
	guiElement->SetBorderGap(borderGap);
	guiElement->SetHoverTexture(hoverTexture);
	guiElement->SetHoverColor(hoverColor);
	guiElement->SetFocusTexture(focusTexture);
	guiElement->SetFocusColor(focusColor);
	guiElement->SetCanHover(canHover);
	guiElement->SetAnimationStyle(GetAnimationStyleFromString(animationType));
	guiElement->SetAnimationTimer(animationDuration);

	XmlElement const* xmlChildElement = xmlElement.FirstChildElement();
	while (xmlChildElement)
	{
		GUI_Element* guiChildElement = GenerateGUIElementFromXmlElement(*xmlChildElement, canvas, guiElement);
		guiElement->AddChild(guiChildElement);
		xmlChildElement = xmlChildElement->NextSiblingElement();
	}

	return guiElement;
}


GUI_AnchorType GUI_Layout::GetAnchorTypeFromString(std::string const& anchorType)
{
	if (anchorType == "topLeft")
	{
		return GUI_AnchorType::TOP_LEFT;
	}
	else if (anchorType == "topCenter")
	{
		return GUI_AnchorType::TOP_CENTER;
	}
	else if (anchorType == "topRight")
	{
		return GUI_AnchorType::TOP_RIGHT;
	}
	else if (anchorType == "middleLeft")
	{
		return GUI_AnchorType::MIDDLE_LEFT;
	}
	else if (anchorType == "center")
	{
		return GUI_AnchorType::CENTER;
	}
	else if (anchorType == "middleRight")
	{
		return GUI_AnchorType::MIDDLE_RIGHT;
	}
	else if (anchorType == "bottomLeft")
	{
		return GUI_AnchorType::BOTTOM_LEFT;
	}
	else if (anchorType == "bottomCenter")
	{
		return GUI_AnchorType::BOTTOM_CENTER;
	}
	else if (anchorType == "bottomRight")
	{
		return GUI_AnchorType::BOTTOM_RIGHT;
	}
	else if (anchorType == "horizontalStretch")
	{
		return GUI_AnchorType::HORIONTAL_STRETCH;
	}
	else if (anchorType == "verticalStretch")
	{
		return GUI_AnchorType::VERTICAL_STRETCH;
	}
	else if (anchorType == "fullStretch")
	{
		return GUI_AnchorType::FULL_STRETCH;
	}
	else
	{
		return GUI_AnchorType::NONE;
	}
}


GUI_BackgroundStyle GUI_Layout::GetBackgroundStyleFromString(std::string const& backgroundStyle)
{
	if (backgroundStyle == "nineSlice")
	{
		return GUI_BackgroundStyle::NINE_SLICE;
	}
	else
	{
		return GUI_BackgroundStyle::DEFAULT;
	}
}


GUI_BorderStyle GUI_Layout::GetBorderStyleFromString(std::string const& borderStyle)
{
	if (borderStyle == "roundedNone")
	{
		return GUI_BorderStyle::ROUNDED_NONE;
	}
	else if (borderStyle == "rect")
	{
		return GUI_BorderStyle::RECT;
	}
	else if (borderStyle == "rounded")
	{
		return GUI_BorderStyle::ROUNDED;
	}
	else if (borderStyle == "doubleRect")
	{
		return GUI_BorderStyle::DOUBLE_RECT;
	}
	else if (borderStyle == "doubleRounded")
	{
		return GUI_BorderStyle::DOUBLE_ROUNDED;
	}
	else if (borderStyle == "singleDivision")
	{
		return GUI_BorderStyle::SINGLE_DIVISION;
	}
	else if (borderStyle == "doubleDivision")
	{
		return GUI_BorderStyle::DOUBLE_DIVISION;
	}
	else if (borderStyle == "doubleDivisionGap")
	{
		return GUI_BorderStyle::DOUBLE_DIVISION_GAP;
	}
	else
	{
		return GUI_BorderStyle::NONE;
	}
}


GUI_AnimationStyle GUI_Layout::GetAnimationStyleFromString(std::string const& animationStyle)
{
	if (animationStyle == "upGlow")
	{
		return GUI_AnimationStyle::UP_GLOW;
	}
	else
	{
		return GUI_AnimationStyle::NONE;
	}
}


