#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>

struct Item;
class Inventory;
class GUI_Canvas;
class Renderer;
class InputSystem;
class BitmapFont;

struct InventorySystemConfig
{
	std::string inventoryID = "";
	std::string inventoryLayoutPath = "";
	std::string buttonEventPath = "";
	Renderer* renderer = nullptr;
	Inventory* inventory = nullptr;
	AABB2 canvasBounds = AABB2::ZERO_TO_ONE;
};

class InventorySystem
{
public:
	InventorySystem(InventorySystemConfig config);
	InventorySystem();
	~InventorySystem();

	void Startup();
	void Update();
	void Render() const;
	void RenderModel() const;
	void Shutdown();

	void ToggleOpen();
	void LoadEvents(std::string const& eventPath);
	void LoadItems(std::string const& itemString);
	void AddItem(Item* item);
	void RemoveItem(int index);
	bool IsOpen();
	void SetScrollableItems(std::string const& scrollableElement, std::string const& fontPath, std::string const& category = "", std::string const& key = "", bool isDescending = false);

private:
	bool Handler_SetModel(EventArgs& args);
	bool Handler_ChangeCategory(EventArgs& args);
	bool Handler_ChangeSort(EventArgs& args);
	bool Handler_FilterUpdate(EventArgs& args);
	bool Handler_ResetCategory(EventArgs& args);
	bool Handler_ResetSort(EventArgs& args);

	//void Handler_SetModel(EventArgs& args);
	//void Handler_ChangeCategory(EventArgs& args);
	//void Handler_ChangeSort(EventArgs& args);
	//void Handler_ClearDisplay(EventArgs& args);

public:
	InventorySystemConfig m_config;
	Renderer* m_renderer = nullptr;
	Inventory* m_inventory = nullptr;
	GUI_Canvas* m_canvas = nullptr;
	std::string m_scrollableElement = "";
	std::string m_scrollableFontPath = "";
	std::string m_searchElement = "";
	std::string m_modelViewerElement = "";
	std::string m_currentCategory = "invalid";
	std::string m_currentSorter = "invalid";
	bool m_isDescend = false;
};


