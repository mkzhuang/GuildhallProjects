#include "Engine/GUI/UILayout.hpp"
#include "Engine/GUI/UIElement.hpp"
#include "Engine/GUI/UIImage.hpp"
#include "Engine/GUI/UIText.hpp"
#include "Engine/GUI/UITextfield.hpp"
#include "Engine/GUI/UIButton.hpp"
#include "Engine/GUI/UIScrollable.hpp"
#include "Engine/GUI/UI3DViewer.hpp"

void UILayout::InitializeLayout(Renderer* renderer, BitmapFont* font)
{
	m_renderer = renderer;
	m_font = font;
}


UIElement* UILayout::CreateLayout(std::string const& layoutFile, AABB2 const& windowBound)
{
	XmlDocument doc;
	doc.LoadFile(layoutFile.c_str());
	XmlElement const* root = doc.RootElement();

	UIElement* uiElement = nullptr;
	if (root)
	{
		uiElement = CreateUIElementFromXmlElement(*root, windowBound);
	}
	return uiElement;
}


UIElement* UILayout::CreateUIElementFromXmlElement(XmlElement const& element, AABB2 const& bounds)
{
	std::string const& type = ParseXmlAttribute(element, "type", "image");
	std::string const& name = ParseXmlAttribute(element, "name", "none");
	Vec2 anchorMins = ParseXmlAttribute(element, "anchorMins", Vec2::ZERO);
	Vec2 anchorMaxs = ParseXmlAttribute(element, "anchorMaxs", Vec2::ONE);
	AABB2 anchor(anchorMins, anchorMaxs);
	bool isDraggable = ParseXmlAttribute(element, "isDraggable", false);

	UIElement* uiElement = nullptr;
	if (type == "image")
	{
		std::string const& imagePath = ParseXmlAttribute(element, "image", "");
		bool hasBorder = ParseXmlAttribute(element, "hasBorder", false);
		Texture* image = imagePath.empty() ? nullptr : m_renderer->CreateOrGetTextureFromFile(imagePath.c_str());
		bool isStretch = ParseXmlAttribute(element, "isStretch", false);
		uiElement = new UIImage(name, bounds, anchor, m_renderer, image, isStretch, AABB2::ZERO_TO_ONE, isDraggable, hasBorder);
	}
	else if (type == "text")
	{
		std::string const& text = ParseXmlAttribute(element, "text", "");
		float textSize = ParseXmlAttribute(element, "size", 32.f);
		Vec2 alignment = ParseXmlAttribute(element, "alignment", Vec2::ZERO);
		uiElement = new UIText(name, bounds, anchor, m_renderer, text, m_font, textSize, alignment, isDraggable);
	}
	else if (type == "textfield")
	{
		std::string const& text = ParseXmlAttribute(element, "text", "");
		float textSize = ParseXmlAttribute(element, "size", 32.f);
		Vec2 alignment = ParseXmlAttribute(element, "alignment", Vec2::ZERO);
		uiElement = new UITextfield(name, bounds, anchor, m_renderer, text, m_font, textSize, alignment, isDraggable);
	}
	else if (type == "button")
	{
		uiElement = new UIButton(name, bounds, anchor, m_renderer, isDraggable);
	}
	else if (type == "scrollable")
	{
 		uiElement = new UIScrollable(name, bounds, anchor, m_renderer, isDraggable);
	}
	else if (type == "3DViewer")
	{
		MeshBuilder meshBuilder;
		std::string const& modelPath = ParseXmlAttribute(element, "model", "");
		std::string const& texturePath = ParseXmlAttribute(element, "texture", "");
		Mat44 transform(Vec3::RIGHT, Vec3::UP, Vec3::FORWARD, Vec3::ZERO);
		MeshBuilderConfig meshBuilderConfig;
		meshBuilderConfig.m_transform = transform;
		meshBuilderConfig.m_scale = 10.f;
		meshBuilderConfig.m_reversedWinding = false;
		meshBuilderConfig.m_invertedTextureV = false;
		meshBuilderConfig.m_modelPath = modelPath;
		meshBuilderConfig.m_texturePath = texturePath;
		uiElement = new UI3DViewer(name, bounds, anchor, m_renderer, meshBuilderConfig, isDraggable);
	}
	else
	{
		ERROR_AND_DIE("Invalid ui element type!");
	}

	XmlElement const* child = element.FirstChildElement();
	while (child)
	{
		UIElement* childUIElement = CreateUIElementFromXmlElement(*child, uiElement->GetScreenWindow());
		uiElement->AddChild(childUIElement);
		child = child->NextSiblingElement();
	}

	return uiElement;
}


