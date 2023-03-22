#include "Game/Chest.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Inventory.hpp"
#include "Game/Item.hpp"
#include "Game/ItemFactory.hpp"
#include "Game/DatabaseClient.hpp"
#include "Engine/GUI/GUI_Canvas.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ProfileLogScope.hpp"
#include "Engine/GUI/GUI_Button.hpp"
#include "Engine/GUI/GUI_Scrollable.hpp"

Chest::Chest()
{
}


Chest::Chest(InventorySystemConfig config)
	: m_config(config)
{
	m_renderer = m_config.renderer;
	m_canvas = new GUI_Canvas(m_config.canvasBounds);
	m_canvas->LoadLayout(m_config.inventoryLayoutPath, m_renderer);
	LoadEvents(m_config.buttonEventPath);

	m_playerInventory = m_config.inventory;
	m_inventoryID = m_config.inventoryID;
	m_chestInventory = new Inventory();

	m_position = Vec3(9.5f, 4.5f, 0.f);
	AddVertsForCylinder3D(m_verts, m_position + Vec3(-0.499f, 0.f, 0.5f), m_position + Vec3(0.4999f, 0.f, 0.5f), 0.5f, 16.f, Rgba8::YELLOW);
	AddVertsForAABB3D(m_verts, AABB3(m_position + Vec3(-0.5f, -0.5f, 0.f), m_position + Vec3(0.5f, 0.5f, 0.5f)), Rgba8::WHITE);
	AddVertsForWireAABB3D(m_verts, AABB3(m_position + Vec3(-0.501f, -0.501f, 0.f), m_position + Vec3(0.501f, 0.501f, 0.501f)), 0.01f, Rgba8::BLACK);

	SetScrollableItems(m_inventoryScrollableElement, m_scrollableFontPath);
}


Chest::~Chest()
{
}


void Chest::Startup()
{

}


void Chest::Update()
{
	m_canvas->Update();
}


void Chest::Render() const
{
	m_canvas->Render(m_renderer);
}


void Chest::RenderModel() const
{
	Rgba8 color = m_isPlayerNear ? Rgba8(255, 100, 100, 50): Rgba8::WHITE;
	g_theRenderer->SetModelMatrix(Mat44());
	g_theRenderer->SetModelColor(color);
	g_theRenderer->DrawVertexArray(m_verts);
	m_renderer->SetModelColor(Rgba8::WHITE);
}


void Chest::Shutdown()
{
}


void Chest::ToggleOpen()
{
	m_canvas->ToggleOpen();
	if (m_canvas->IsOpen())
	{
		SetScrollableItems(m_inventoryScrollableElement, m_scrollableFontPath, m_currentInventoryCategory);
		SetScrollableItems(m_chestScrollableElement, m_scrollableFontPath, m_currentChestCategory);
	}
}


void Chest::LoadItems(std::string const& itemFile)
{
	XmlDocument doc;
	doc.LoadFile(itemFile.c_str());
	XmlElement const* root = doc.RootElement();

	XmlElement const* child = root->FirstChildElement();

	while (child)
	{
		std::string itemName = ParseXmlAttribute(*child, "name", "");
		Item* item = ItemFactory::CreateByName(itemName);
		AddToChest(item);
		child = child->NextSiblingElement();
	}
}


void Chest::LoadItemsFromJsonString(std::string const& itemString)
{
	Strings itemCount = SplitStringOnDelimiter(itemString, ',');
	for (std::string itemID : itemCount)
	{
		AddToChest(ItemFactory::CreateById((uint16_t)std::atoi(itemID.c_str())));
	}

	SetScrollableItems(m_chestScrollableElement, m_scrollableFontPath);
}


void Chest::LoadEvents(std::string const& eventPath)
{
	XmlDocument doc;
	doc.LoadFile(eventPath.c_str());
	XmlElement const* root = doc.RootElement();

	XmlElement const* child = root->FirstChildElement();
	if (strcmp(child->Name(), "InventoryScrollable") == 0)
	{
		m_inventoryScrollableElement = ParseXmlAttribute(*child, "name", "");
		m_scrollableFontPath = ParseXmlAttribute(*child, "fontPath", "");
		child = child->NextSiblingElement();
	}

	if (strcmp(child->Name(), "ChestScrollable") == 0)
	{
		m_chestScrollableElement = ParseXmlAttribute(*child, "name", "");
		m_scrollableFontPath = ParseXmlAttribute(*child, "fontPath", "");
		child = child->NextSiblingElement();
	}

	if (strcmp(child->Name(), "ItemCategories") == 0)
	{
		std::string const& categories = ParseXmlAttribute(*child, "value", "");
		m_itemCategories = SplitStringOnDelimiter(categories, ',');
		m_itemCategories.insert(m_itemCategories.begin(), "");
		child = child->NextSiblingElement();
	}

	if (strcmp(child->Name(), "ItemCategoriesText") == 0)
	{
		std::string const& texts = ParseXmlAttribute(*child, "value", "");
		m_categoryTexts = SplitStringOnDelimiter(texts, ',');
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

		if (funcName == "ShiftCategory")
		{
			RegisterEventHandler(eventName, this, &Chest::Handler_ShiftCategory, args);
		}

		if (!buttonName.empty())
		{
			GUI_Element* button = m_canvas->FindElementByName(buttonName);
			static_cast<GUI_Button*>(button)->SetHandler(eventName);
		}

		child = child->NextSiblingElement();
	}
}


void Chest::AddFromPlayerInventory(int index)
{
	Item* item = m_playerInventory->RemoveItemFromInventory(index);
	m_chestInventory->AddItemToInventory(item);
	g_theClient->RemoveItemFromInventory(m_playerInventory->m_inventoryID, std::to_string(item->GetID()));
	g_theClient->AddItemToInventory(m_inventoryID, std::to_string(item->GetID()));
}


void Chest::RemoveFromChest(int index)
{
	Item const* item = m_chestInventory->GetItemByIndex(index);
	if (!m_playerInventory->CanAddItemWithWeight(item->GetValueAsFloat("weight")))
	{
		g_theDevConsole->AddLine(DevConsole::INFO_WARNING, "Can't add more items");
		return;
	}

	Item* removeItem = m_chestInventory->RemoveItemFromInventory(index);
	m_playerInventory->AddItemToInventory(removeItem);
	g_theClient->RemoveItemFromInventory(m_inventoryID, std::to_string(removeItem->GetID()));
	g_theClient->AddItemToInventory(m_playerInventory->m_inventoryID, std::to_string(removeItem->GetID()));
}


void Chest::AddToChest(Item* item)
{
	m_chestInventory->AddItemToInventory(item);
}


bool Chest::IsOpen()
{
	return m_canvas->IsOpen();
}


void Chest::SetScrollableItems(std::string const& scrollableElement, std::string const& fontPath, std::string const& category, std::string const& key, bool isDescending)
{
	Inventory* inventory = m_playerInventory;
	if (scrollableElement == m_chestScrollableElement)
	{
		inventory = m_chestInventory;
	}
	ProfileLogScope profiler("Inventory Filtering");
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont(fontPath.c_str());
	std::vector<Item*> items = inventory->SortBy(category, key, isDescending);

	GUI_Element* scrollable = m_canvas->FindElementByName(scrollableElement);
	static_cast<GUI_Scrollable*>(scrollable)->ClearEntries();

	std::vector<GUI_Element*> buttons;
	for (int index = 0; index < (int)items.size(); index++)
	{
		Item*& item = items[index];
		std::string const& buttonName = Stringf("%sitemButton%d", scrollableElement.c_str(), index);
		std::string itemName = item->GetValueAsString("name");

		GUI_Button* button = new GUI_Button();
		button->SetCanvas(m_canvas);
		button->SetName(buttonName);
		button->SetAnchorType(GUI_AnchorType::NONE);
		button->SetText(itemName);
		button->SetTextHeight(16.f);
		button->SetFont(font);
		button->SetHasDropShadow(true);
		button->SetTextAlignment(Vec2(0.02f, 0.5f));
		button->SetHoverColor(Rgba8(200, 200, 200, 255));
		button->SetFocusColor(Rgba8(100, 100, 100, 255));

		buttons.push_back(button);

		//add button event
		int itemIndex = inventory->GetIndexByName(itemName);
		EventArgs args;
		std::string from = scrollableElement == m_inventoryScrollableElement ? "player" : "chest";
		args.SetValue("from", from);
		args.SetValue("index", itemIndex);
		RegisterEventHandler(buttonName, this, &Chest::Handler_MoveItem, args);
		button->SetHandler(buttonName);
	}

	static_cast<GUI_Scrollable*>(scrollable)->SetEntries(buttons);
}


bool Chest::Handler_MoveItem(EventArgs& args)
{
	std::string from = args.GetValue("from", "");
	int itemIndex = args.GetValue("index", -1);

	if (from == "player")
	{
		AddFromPlayerInventory(itemIndex);
	}
	else if (from == "chest")
	{
		RemoveFromChest(itemIndex);
	}

	SetScrollableItems(m_inventoryScrollableElement, m_scrollableFontPath, m_currentInventoryCategory);
	SetScrollableItems(m_chestScrollableElement, m_scrollableFontPath, m_currentChestCategory);

	m_canvas->ResetHoverAndFocus();

	return false;
}


bool Chest::Handler_ShiftCategory(EventArgs& args)
{
	std::string const& scrollable = args.GetValue("scrollable", "");
	std::string const& changeDirectionText = args.GetValue("changeDirection", "");
	int changeDirection = stoi(changeDirectionText);
	std::string category = "";
	if (scrollable == m_inventoryScrollableElement)
	{
		category = m_currentInventoryCategory;
	}
	else if (scrollable == m_chestScrollableElement)
	{
		category = m_currentChestCategory;
	}

	int index = 0;
	for (index; index < (int)m_itemCategories.size(); index++)
	{
		if (category == m_itemCategories[index])
		{
			index += changeDirection;
			if (index == -1)
			{
				index = (int)m_itemCategories.size() - 1;	
			}
			else if (index == (int)m_itemCategories.size())
			{
				index = 0;
			}
			category = m_itemCategories[index];
			break;
		}
	}

	if (scrollable == m_inventoryScrollableElement)
	{
		m_currentInventoryCategory = category;
	}
	else if (scrollable == m_chestScrollableElement)
	{
		m_currentChestCategory = category;
	}

	std::string const& title = args.GetValue("title", "");
	std::string const& titleText = m_categoryTexts[index];

	GUI_Element* titleElement = m_canvas->FindElementByName(title);
	dynamic_cast<GUI_Text*>(titleElement)->SetText(titleText);

	SetScrollableItems(scrollable, m_scrollableFontPath, category);
	return false;
}


