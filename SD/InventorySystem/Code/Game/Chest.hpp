#pragma once
#include "Game/InventorySystem.hpp"
#include "Engine/Core/EngineCommon.hpp"

class Chest
{
public:
	Chest(InventorySystemConfig config);
	Chest();
	~Chest();

	void Startup();
	void Update();
	void Render() const;
	void RenderModel() const;
	void Shutdown();

	void ToggleOpen();
	void LoadItems(std::string const& itemFile);
	void LoadItemsFromJsonString(std::string const& itemString);
	void LoadEvents(std::string const& eventPath);
	void AddFromPlayerInventory(int index);
	void RemoveFromChest(int index);
	void AddToChest(Item* item);
	bool IsOpen();
	void SetScrollableItems(std::string const& scrollableElement, std::string const& fontPath, std::string const& category = "", std::string const& key = "", bool isDescending = false);

	bool Handler_MoveItem(EventArgs& args);
	bool Handler_ShiftCategory(EventArgs& args);

public:
	Vec3 m_position = Vec3::ZERO;
	std::vector<Vertex_PCU> m_verts;
	bool m_isPlayerNear = false;
	InventorySystemConfig m_config;
	Renderer* m_renderer = nullptr;
	std::string m_inventoryID = "";
	Inventory* m_playerInventory = nullptr;
	Inventory* m_chestInventory = nullptr;
	GUI_Canvas* m_canvas = nullptr;
	std::string m_chestScrollableElement = "";
	std::string m_inventoryScrollableElement = "";
	std::string m_currentInventoryCategory = "";
	std::string m_currentChestCategory = "";
	std::string m_scrollableFontPath = "";
	std::vector<std::string> m_itemCategories;
	std::vector<std::string> m_categoryTexts;

	bool m_wasUpdateLastFrame = false;
};


