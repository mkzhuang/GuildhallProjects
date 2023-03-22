#pragma once
#include "Game/InventorySystem.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <string>
#include <vector>

class WorldInventory
{
public:
	WorldInventory(InventorySystemConfig config);
	~WorldInventory();

	void Update(float deltaSeconds);
	void Render() const;

	void LoadItemsFromJsonString(std::string const& itemString);
	void LoadItemPosFromJsonString(std::string const& posString);
	void AddFromPlayerInventory(int index, Vec2 const& itemPos);
	void RemoveFromWorldInventory(int index);
	void AddToWorld(Item* item);
	void AddWorldLocation(Vec2 const& pos);
	Vec2 const& RemoveWorldLocation(int index);

public:
	Inventory* m_worldInventory = nullptr;
	Inventory* m_playerInventory = nullptr;
	Renderer* m_renderer = nullptr;
	std::string m_inventoryID = "";
	std::vector<Vec2> m_itemPos;
};