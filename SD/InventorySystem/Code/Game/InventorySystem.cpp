#include "Game/InventorySystem.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Inventory.hpp"
#include "Game/Item.hpp"
#include "Game/ItemFactory.hpp"
#include "Engine/Mesh/MeshBuilder.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ProfileLogScope.hpp"
#include "Engine/GUI/GUI_Canvas.hpp"
#include "Engine/GUI/GUI_Button.hpp"
#include "Engine/GUI/GUI_Scrollable.hpp"
#include "Engine/GUI/GUI_Textfield.hpp"
#include "Engine/GUI/GUI_ModelViewer.hpp"

InventorySystem::InventorySystem()
{
}


InventorySystem::InventorySystem(InventorySystemConfig config)
	: m_config(config)
{
	m_renderer = m_config.renderer;
	m_inventory = m_config.inventory;
	m_canvas = new GUI_Canvas(m_config.canvasBounds);
	m_canvas->LoadLayout(m_config.inventoryLayoutPath, m_renderer);
	LoadEvents(m_config.buttonEventPath);

	m_inventory = new Inventory();
	m_inventory->m_inventoryID = m_config.inventoryID;

	CallHandler("allCategory");
	CallHandler("nameSort");
}


InventorySystem::~InventorySystem()
{
	delete m_canvas;
}


void InventorySystem::Startup()
{
}


void InventorySystem::Update()
{
	m_canvas->Update();

}


void InventorySystem::Render() const
{
	m_canvas->Render(m_renderer);
}


void InventorySystem::RenderModel() const
{
	m_canvas->RenderModel(m_renderer);
}

void InventorySystem::Shutdown()
{
}


void InventorySystem::ToggleOpen()
{
	m_canvas->ToggleOpen();
	if (m_canvas->IsOpen())
	{
		SetScrollableItems(m_scrollableElement, m_scrollableFontPath, m_currentCategory, m_currentSorter, m_isDescend);
	}
}


void InventorySystem::LoadEvents(std::string const& eventPath)
{
	XmlDocument doc;
	doc.LoadFile(eventPath.c_str());
	XmlElement const* root = doc.RootElement();

	XmlElement const* child = root->FirstChildElement();
	if (strcmp(child->Name(), "InventoryScrollable") == 0)
	{
		m_scrollableElement = ParseXmlAttribute(*child, "name", "");
		m_scrollableFontPath = ParseXmlAttribute(*child, "fontPath", "");
		child = child->NextSiblingElement();
	}

	if (strcmp(child->Name(), "SearchTextfield") == 0)
	{
		m_searchElement = ParseXmlAttribute(*child, "name", "");
		child = child->NextSiblingElement();
	}

	if (strcmp(child->Name(), "ModelViewer") == 0)
	{
		m_modelViewerElement = ParseXmlAttribute(*child, "name", "");
		child = child->NextSiblingElement();
	}

	while (child)
	{
		std::string const& eventName = ParseXmlAttribute(*child, "name", "");
		std::string const& funcName = ParseXmlAttribute(*child, "function", "");
		std::string const& buttonName = ParseXmlAttribute(*child, "button", "");

		EventArgs args;
		XmlElement const* arg = child->FirstChildElement();
		while (arg)
		{
			std::string const& key = ParseXmlAttribute(*arg, "key", "");
			std::string const& value = ParseXmlAttribute(*arg, "value", "");
			args.SetValue(key, value);
			arg = arg->NextSiblingElement();
		}

		if (funcName == "ChangeCategory")
		{
			RegisterEventHandler(eventName, this, &InventorySystem::Handler_ChangeCategory, args);
		}
		else if (funcName == "ChangeSort")
		{
			RegisterEventHandler(eventName, this, &InventorySystem::Handler_ChangeSort, args);
		}
		else if (funcName == "FilterUpdate")
		{
			RegisterEventHandler(eventName, this, &InventorySystem::Handler_FilterUpdate, args);
			GUI_Element* textfield = m_canvas->FindElementByName(m_searchElement);
			static_cast<GUI_Textfield*>(textfield)->SetHandler(eventName);
		}
		else if (funcName == "ResetCategory")
		{
			RegisterEventHandler(eventName, this, &InventorySystem::Handler_ResetCategory, args);
		}
		else if (funcName == "ResetSort")
		{
			RegisterEventHandler(eventName, this, &InventorySystem::Handler_ResetSort, args);
		}


		if (!buttonName.empty())
		{
			GUI_Element* button = m_canvas->FindElementByName(buttonName);
			static_cast<GUI_Button*>(button)->SetHandler(eventName);
		}

		child = child->NextSiblingElement();
	}
}


void InventorySystem::LoadItems(std::string const& itemString)
{
	Strings itemCount = SplitStringOnDelimiter(itemString, ',');
	for (std::string itemID : itemCount)
	{
		AddItem(ItemFactory::CreateById((uint16_t)std::atoi(itemID.c_str())));
	}

	SetScrollableItems(m_scrollableElement, m_scrollableFontPath);
}


void InventorySystem::AddItem(Item* item)
{
	m_inventory->AddItemToInventory(item);
}


void InventorySystem::RemoveItem(int index)
{
	m_inventory->RemoveItemFromInventory(index);
}


bool InventorySystem::IsOpen()
{
	return m_canvas->IsOpen();
}


void InventorySystem::SetScrollableItems(std::string const& scrollableElement, std::string const& fontPath, std::string const& category, std::string const& key, bool isDescending)
{
	ProfileLogScope profiler("Inventory Filtering");
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont(fontPath.c_str());
	std::vector<Item*> items = m_inventory->SortBy(category, key, isDescending);

	GUI_Element* textfield = m_canvas->FindElementByName(m_searchElement);
	if (textfield)
	{
		std::string filterText = static_cast<GUI_Textfield*>(textfield)->GetText();
		if (!filterText.empty())
		{
			for (std::vector<Item*>::iterator iter = items.begin(); iter != items.end(); )
			{
				std::string itemName = (*iter)->GetValueAsString("name");
				for (char& c : filterText) { c = (char)tolower(c); }
				for (char& c : itemName)   { c = (char)tolower(c); }
				if (!ContainsSubstring(itemName, filterText))
				{
					iter = items.erase(iter);
				}
				else
				{
					iter++;
				}
			}
		}
	}

	GUI_Element* scrollable = m_canvas->FindElementByName(scrollableElement);
	static_cast<GUI_Scrollable*>(scrollable)->ClearEntries();

	std::vector<GUI_Element*> buttons;
	for (int index = 0; index < (int)items.size(); index++)
	{
		Item*& item = items[index];
		std::string const& buttonName = Stringf("itemButton%d", index);
		std::string itemName = item->GetValueAsString("name");
		std::string itemWeight = Stringf("%.2f", item->GetValueAsFloat("weight"));
		std::string itemValue = Stringf("%.2f", item->GetValueAsFloat("value"));


		Texture* itemArrow = m_renderer->CreateOrGetTextureFromFile("Data/Images/UI/item_arrow.png");
		GUI_Button* button = new GUI_Button();
		button->SetCanvas(m_canvas);
		button->SetName(buttonName);
		button->SetAnchorType(GUI_AnchorType::NONE);
		button->SetHoverTexture(itemArrow);
		button->SetHoverColor(Rgba8(255, 255, 255, 100));
		button->SetFocusTexture(itemArrow);
		button->SetFocusColor(Rgba8(255, 255, 255, 200));

		GUI_Text* nameText = new GUI_Text();
		nameText->SetCanvas(m_canvas);
		nameText->SetName(buttonName + "name");
		nameText->SetAnchorType(GUI_AnchorType::NONE);
		nameText->SetText(itemName);
		nameText->SetTextHeight(16.f);
		nameText->SetFont(font);
		nameText->SetHasDropShadow(true);
		nameText->SetTextAlignment(Vec2(0.02f, 0.5f));
		nameText->SetHoverColor(Rgba8(200, 200, 200, 255));
		nameText->SetCanHover(false);
		nameText->SetIsParentRelative(true, AABB2(Vec2(0.f, 0.f), Vec2(0.51f, 1.f)));
		button->AddChild(nameText);

		GUI_Text* weightText = new GUI_Text();
		weightText->SetCanvas(m_canvas);
		weightText->SetName(buttonName + "weight");
		weightText->SetAnchorType(GUI_AnchorType::NONE);
		weightText->SetText(itemWeight);
		weightText->SetTextHeight(16.f);
		weightText->SetFont(font);
		weightText->SetHasDropShadow(true);
		weightText->SetTextAlignment(Vec2(0.02f, 0.5f));
		weightText->SetHoverColor(Rgba8(200, 200, 200, 255));
		weightText->SetCanHover(false);
		weightText->SetIsParentRelative(true, AABB2(Vec2(0.51f, 0.f), Vec2(0.77f, 1.f)));
		button->AddChild(weightText);

		GUI_Text* valueText = new GUI_Text();
		valueText->SetCanvas(m_canvas);
		valueText->SetName(buttonName + "value");
		valueText->SetAnchorType(GUI_AnchorType::NONE);
		valueText->SetText(itemValue);
		valueText->SetTextHeight(16.f);
		valueText->SetFont(font);
		valueText->SetHasDropShadow(true);
		valueText->SetTextAlignment(Vec2(0.02f, 0.5f));
		valueText->SetHoverColor(Rgba8(200, 200, 200, 255));
		valueText->SetCanHover(false);
		valueText->SetIsParentRelative(true, AABB2(Vec2(0.77f, 0.f), Vec2(1.f, 1.f)));
		button->AddChild(valueText);

		buttons.push_back(button);

		//add button event
		EventArgs args;
		args.SetValue("model", item->GetValueAsString("model"));
		args.SetValue("texture", item->GetValueAsString("texture"));
		args.SetValue("scale", std::to_string(item->GetValueAsFloat("scale")));
		args.SetValue("name", itemName);
		args.SetValue("weight", itemWeight);
		args.SetValue("value", itemValue);
		RegisterEventHandler(buttonName, this, &InventorySystem::Handler_SetModel, args);
		button->SetHandler(buttonName);
	}

	static_cast<GUI_Scrollable*>(scrollable)->SetEntries(buttons);
}


bool InventorySystem::Handler_SetModel(EventArgs& args)
{
	MeshBuilder meshBuilder;
	std::string const& modelName = args.GetValue("name", "");
	std::string const& modelPath = args.GetValue("model", "");
	std::string const& texturePath = args.GetValue("texture", "");
	float modelScale = args.GetValue("scale", 1.f);
	Mat44 transform(Vec3::FORWARD, Vec3::RIGHT, Vec3::UP, Vec3::ZERO);
	MeshBuilderConfig meshBuilderConfig;
	meshBuilderConfig.m_transform = transform;
	meshBuilderConfig.m_scale = modelScale;
	meshBuilderConfig.m_reversedWinding = false;
	meshBuilderConfig.m_invertedTextureV = false;
	meshBuilderConfig.m_modelPath = modelPath;
	meshBuilderConfig.m_texturePath = texturePath;
	Mesh* mesh = m_renderer->CreateOrGetMeshFromConfig(modelName.c_str(), meshBuilderConfig);
	GUI_Element* modelViewer = m_canvas->FindElementByName(m_modelViewerElement);
	static_cast<GUI_ModelViewer*>(modelViewer)->SetMesh(mesh);
	static_cast<GUI_ModelViewer*>(modelViewer)->SetupCamera();

	return false;
}


bool InventorySystem::Handler_ChangeCategory(EventArgs& args)
{
	std::string const& category = args.GetValue("category", "");
	if (m_currentCategory == category) return false;
	m_currentCategory = category;
	std::string handler = args.GetValue("handler", "");
	std::string const& categoryImage = args.GetValue("image", "");
	std::string const& categoryImagePath = args.GetValue("imagePath", "");
	std::string categoryColorString = args.GetValue("imageColor", "");
	std::string const& divisionLine = args.GetValue("divisionLine", "");
	std::string const& divisionGapString = args.GetValue("divisionGap", "");
	std::string const& title = args.GetValue("title", "");
	std::string const& titleText = args.GetValue("titleText", "");
	Vec2 divisionGap;
	divisionGap.SetFromText(divisionGapString);

	Rgba8 categoryImageColor;
	categoryImageColor.SetFromText(categoryColorString);

	CallHandler(handler);
	GUI_Element* titleElement = m_canvas->FindElementByName(title);
	dynamic_cast<GUI_Text*>(titleElement)->SetText(titleText);

	GUI_Element* element = m_canvas->FindElementByName(categoryImage);
	Texture* texture = m_renderer->CreateOrGetTextureFromFile(categoryImagePath.c_str());
	element->SetBackgroundTexture(texture);
	element->SetBackgroundColor(categoryImageColor);

	GUI_Element* division = m_canvas->FindElementByName(divisionLine);
	division->SetBorderGap(divisionGap);
	SetScrollableItems(m_scrollableElement, m_scrollableFontPath, m_currentCategory, m_currentSorter, m_isDescend);
	return false;
}


bool InventorySystem::Handler_ChangeSort(EventArgs& args)
{
	std::string const& sorter = args.GetValue("sorter", "");
	std::string const& handler = args.GetValue("handler", "");
	std::string const& orderImage = args.GetValue("image", "");
	std::string const& ascendImagePath = args.GetValue("ascendImagePath", "");
	std::string const& descendImagePath = args.GetValue("descendImagePath", "");
	std::string const& ascendColorString = args.GetValue("ascendColor", "");
	Rgba8 ascendColor;
	ascendColor.SetFromText(ascendColorString);
	std::string const& descendColorString = args.GetValue("descendColor", "");
	Rgba8 descendColor;
	descendColor.SetFromText(descendColorString);

	if (m_currentSorter != sorter)
	{
		m_isDescend = false;
		m_currentSorter = sorter;
	}
	else
	{
		m_isDescend = !m_isDescend;
	}
	CallHandler(handler);
	GUI_Element* element = m_canvas->FindElementByName(orderImage);
	if (!m_isDescend)
	{
		Texture* texture = m_renderer->CreateOrGetTextureFromFile(ascendImagePath.c_str());
		element->SetBackgroundTexture(texture);
		element->SetBackgroundColor(ascendColor);
	}
	else
	{
		Texture* texture = m_renderer->CreateOrGetTextureFromFile(descendImagePath.c_str());
		element->SetBackgroundTexture(texture);
		element->SetBackgroundColor(descendColor);
	}
	SetScrollableItems(m_scrollableElement, m_scrollableFontPath, m_currentCategory, m_currentSorter, m_isDescend);
	return false;
}


bool InventorySystem::Handler_FilterUpdate(EventArgs& args)
{
	UNUSED(args)
	SetScrollableItems(m_scrollableElement, m_scrollableFontPath, m_currentCategory, m_currentSorter, m_isDescend);
	return false;
}

bool InventorySystem::Handler_ResetCategory(EventArgs& args)
{
	std::string const& elementNames = args.GetValue("elements", "");
	Strings elements = SplitStringOnDelimiter(elementNames, ',');
	std::string const& imagePaths = args.GetValue("images", "");
	Strings images = SplitStringOnDelimiter(imagePaths, ',');
	std::string const& imageColorString = args.GetValue("color", "");
	Rgba8 color;
	color.SetFromText(imageColorString);

	for (int index = 0; index < (int)elements.size(); index++)
	{
		GUI_Element* element = m_canvas->FindElementByName(elements[index]);
		Texture* texture = m_renderer->CreateOrGetTextureFromFile(images[index].c_str());
		element->SetBackgroundTexture(texture);
		element->SetBackgroundColor(color);
	}
	return false;
}

bool InventorySystem::Handler_ResetSort(EventArgs& args)
{
	std::string const& elementNames = args.GetValue("elements", "");
	Strings elements = SplitStringOnDelimiter(elementNames, ',');
	std::string const& imageColorString = args.GetValue("color", "");
	Rgba8 color;
	color.SetFromText(imageColorString);

	for (int index = 0; index < (int)elements.size(); index++)
	{
		GUI_Element* element = m_canvas->FindElementByName(elements[index]);
		element->SetBackgroundTexture(nullptr);
		element->SetBackgroundColor(color);
	}
	return false;
}


//void InventorySystem::SetDisplayItems(std::string const& displayElement, std::string const& category, std::string const& key, bool isDescend, size_t topItemIndex)
//{
//	ProfileLogScope profiler("Inventory View Regen");
//	UIElement* element = m_uiSystem->m_canvas->FindElementByName(m_uiSystem->m_canvas->m_rootElement, displayElement);
//	std::vector<UIElement*>& childElements = element->m_children;
//	for (UIElement*& child : childElements)
//	{
//		if (dynamic_cast<UIButton*>(child))
//		{
//			m_uiSystem->m_canvas->RemoveHandler(child->m_name);
//			UnregisterEventHandler(child->m_name);
//		}
//		delete child;
//	}
//	childElements.clear();
//
//	BitmapFont* textFont = m_config.font;
//	std::vector<Item*> allItems = m_inventory->SortBy(category, key, isDescend);
//	std::vector<Item*> items;
//	UIElement* textfield = m_uiSystem->m_canvas->FindElementByName(m_uiSystem->m_canvas->m_rootElement, "searchField");
//	std::string const& text = dynamic_cast<UITextfield*>(textfield)->GetText();
//	for (Item* item : allItems)
//	{
//		std::string itemName = item->GetValueAsString("name");
//		if (ContainsSubstring(itemName, text))
//		{
//			items.push_back(item);
//		}
//	}
//	static float itemPercentage = 0.1f;
//	int maxItemIndex = (int)items.size();
//	UIElement* scrollable = m_uiSystem->m_canvas->FindElementByName(m_uiSystem->m_canvas->m_rootElement, "itemScrollable");
//	dynamic_cast<UIScrollable*>(scrollable)->SetTotalEntries(items.size());
//	dynamic_cast<UIScrollable*>(scrollable)->SetDisplayableEntries(10);
//	if (topItemIndex + 10 < maxItemIndex)
//	{
//		maxItemIndex = (int)topItemIndex + 10;
//	}
//	for (int itemIndex = (int)topItemIndex; itemIndex < maxItemIndex; itemIndex++)
//	{
//		Item*& item = items[itemIndex];
//		float heightOffset = static_cast<float>(itemIndex - (int)topItemIndex) * itemPercentage;
//		UIAnchor anchorItem(AABB2(Vec2(0.f, 0.9f - heightOffset), Vec2(1.f, 1.f - heightOffset)));
//		std::string buttonName = Stringf("itemButton%d", itemIndex);
//		UIButton* itemButton = new UIButton(buttonName, element->m_screenWindow, anchorItem, m_config.renderer);
//		element->AddChild(itemButton);
//		UIAnchor anchorItemImage(AABB2(Vec2(0.f, 0.1f), Vec2(1.f, 0.9f)));
//		std::string imageName = Stringf("itemImage%d", itemIndex);
//		UIImage* itemNameImage = new UIImage(imageName, itemButton->m_screenWindow, anchorItemImage, m_config.renderer);
//		itemButton->AddChild(itemNameImage);
//		UIAnchor anchorItemName(AABB2(Vec2(0.025f, 0.f), Vec2(0.5f, 1.f)));
//		std::string itemTextName = Stringf("itemName%d", itemIndex);
//		std::string itemName = item->GetValueAsString("name");
//		UIText* itemNameText = new UIText(itemTextName, itemButton->m_screenWindow, anchorItemName, m_config.renderer, itemName, textFont, 24.f, Vec2(0.f, 0.5f));
//		itemButton->AddChild(itemNameText);
//		UIAnchor anchorItemWeight(AABB2(Vec2(0.525f, 0.f), Vec2(0.75f, 1.f)));
//		std::string itemTextWeight = Stringf("itemWeight%d", itemIndex);
//		std::string itemWeight = Stringf("%.2f", item->GetValueAsFloat("weight"));
//		UIText* itemWeightText = new UIText(itemTextWeight, itemButton->m_screenWindow, anchorItemWeight, m_config.renderer, itemWeight, textFont, 24.f, Vec2(0.f, 0.5f));
//		itemButton->AddChild(itemWeightText);
//		UIAnchor anchorItemValue(AABB2(Vec2(0.775f, 0.f), Vec2(1.f, 1.f)));
//		std::string itemTextValue = Stringf("itemValue%d", itemIndex);
//		std::string itemValue = Stringf("%.2f", item->GetValueAsFloat("value"));
//		UIText* itemValueText = new UIText(itemTextValue, itemButton->m_screenWindow, anchorItemValue, m_config.renderer, itemValue, textFont, 24.f, Vec2(0.f, 0.5f));
//		itemButton->AddChild(itemValueText);
//		m_uiSystem->m_canvas->AddHandler(buttonName);
//		EventArgs args;
//		args.SetValue("model", item->GetValueAsString("model"));
//		args.SetValue("texture", item->GetValueAsString("texture"));
//		args.SetValue("scale", std::to_string(item->GetValueAsFloat("scale")));
//		RegisterEventHandler(buttonName, this, &InventorySystem::Handler_SetModel, args);
//		itemButton->SetHandler(buttonName);
//	}
//
//	if (!items.empty())
//	{
//		Item*& firstItem = items[topItemIndex];
//		EventArgs args;
//		args.SetValue("model", firstItem->GetValueAsString("model"));
//		args.SetValue("texture", firstItem->GetValueAsString("texture"));
//		args.SetValue("scale", std::to_string(firstItem->GetValueAsFloat("scale")));
//		Handler_SetModel(args);
//	}
//	else
//	{
//		EventArgs args;
//		args.SetValue("model", "");
//		args.SetValue("texture", "");
//		args.SetValue("scale", "");
//		Handler_SetModel(args);
//	}
//
//	element->Update();
//}
//
//
//void InventorySystem::Handler_SetModel(EventArgs& args)
//{
//	MeshBuilder meshBuilder;
//	std::string const& modelPath = args.GetValue("model", "");
//	std::string const& texturePath = args.GetValue("texture", "");
//	float modelScale = args.GetValue("scale", 1.f);
//	Mat44 transform(Vec3::FORWARD, Vec3::RIGHT, Vec3::UP, Vec3::ZERO);
//	MeshBuilderConfig meshBuilderConfig;
//	meshBuilderConfig.m_transform = transform;
//	meshBuilderConfig.m_scale = modelScale;
//	meshBuilderConfig.m_reversedWinding = false;
//	meshBuilderConfig.m_invertedTextureV = false;
//	meshBuilderConfig.m_modelPath = modelPath;
//	meshBuilderConfig.m_texturePath = texturePath;
//	UIElement* viewer = m_uiSystem->m_canvas->FindViewerElement(m_uiSystem->m_canvas->m_rootElement);
//	dynamic_cast<UI3DViewer*>(viewer)->SetMesh(meshBuilderConfig);
//}
//
//
//void InventorySystem::Handler_ChangeCategory(EventArgs& args)
//{
//	std::string displayElement = args.GetValue("display", "");
//	std::string category = args.GetValue("category", "");
//	m_currentCategory = category;
//	SetDisplayItems(displayElement, m_currentCategory, m_currentSorter, m_isDescend);
//}
//
//
//void InventorySystem::Handler_ChangeSort(EventArgs& args)
//{
//	std::string displayElement = args.GetValue("display", "");
//	std::string sorter = args.GetValue("sorter", "");
//	std::string handler = args.GetValue("handler", "");
//	//std::string elementText = args.GetValue("element", "");
//	//std::string text = args.GetValue("text", "");
//	std::string image = args.GetValue("image", "");
//	if (m_currentSorter != sorter)
//	{
//		m_isDescend = false;
//		m_currentSorter = sorter;
//	}
//	else
//	{
//		m_isDescend = !m_isDescend;
//	}
//	CallHandler(handler);
//	UIElement* element = m_uiSystem->m_canvas->FindElementByName(m_uiSystem->m_canvas->m_rootElement, image);
//	std::string imagePath = m_isDescend ? "Data/Images/UI/arrow_down.png" : "Data/Images/UI/arrow_up.png";
//	Texture* texture = m_config.renderer->CreateOrGetTextureFromFile(imagePath.c_str());
//	dynamic_cast<UIImage*>(element)->SetTexture(texture);
//	dynamic_cast<UIImage*>(element)->Update();
//	//UIElement* element = m_uiSystem->m_canvas->FindElementByName(m_uiSystem->m_canvas->m_rootElement, elementText);
//	//text = m_isDescend ? text + " d" : text + " a";
//	//dynamic_cast<UIText*>(element)->SetText(text);
//	SetDisplayItems(displayElement, m_currentCategory, m_currentSorter, m_isDescend);
//}
//
//
//void InventorySystem::Handler_ClearDisplay(EventArgs& args)
//{
//	std::string elementsName = args.GetValue("elements", "");
//	Strings elements = SplitStringOnDelimiter(elementsName, ',');
//	std::string elementsText = args.GetValue("texts", "");
//	Strings texts = SplitStringOnDelimiter(elementsText, ',');
//	std::string imagesName = args.GetValue("images", "");
//	Strings images = SplitStringOnDelimiter(imagesName, ',');
//
//	for (size_t index = 0; index < elements.size(); index++)
//	{
//		UIElement* element = m_uiSystem->m_canvas->FindElementByName(m_uiSystem->m_canvas->m_rootElement, elements[index]);
//		dynamic_cast<UIText*>(element)->SetText(texts[index]);
//	}
//
//	for (size_t index = 0; index < images.size(); index++)
//	{
//		UIElement* element = m_uiSystem->m_canvas->FindElementByName(m_uiSystem->m_canvas->m_rootElement, images[index]);
//		dynamic_cast<UIImage*>(element)->SetTexture(nullptr);
//		dynamic_cast<UIImage*>(element)->Update();
//	}
//}


