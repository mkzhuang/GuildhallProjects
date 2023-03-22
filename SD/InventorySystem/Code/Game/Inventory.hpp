#pragma once

#include <vector>
#include <string>
#include <functional>

struct Item;

class Inventory
{
public:
	Inventory();
	~Inventory();

	void SetMaxWeight(float weight);
	float GetCurrentWeight() const;
	bool CanAddItemWithWeight(float weight);
	void AddItemToInventory(Item* item);
	Item* RemoveItemFromInventory(int index);
	Item const* GetItemByIndex(int index) const;
	int GetIndexByName(std::string const& name);
	std::vector<Item*> SortBy(std::string const& category, std::string const& itemKey = "", bool isDescend = false);

public:
	std::string m_inventoryID = "";
	std::vector<Item*> m_items;
	float m_maxWeight = 99999999.f;
};


