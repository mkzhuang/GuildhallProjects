#include "Game/Inventory.hpp"
#include "Game/Filter.hpp"
#include "Game/Item.hpp"

Inventory::Inventory()
{

}


Inventory::~Inventory()
{
	for (Item*& item : m_items)
	{
		delete item;
	}
	m_items.clear();
}


void Inventory::SetMaxWeight(float weight)
{
	m_maxWeight = weight;
}


float Inventory::GetCurrentWeight() const
{
	float weight = 0.f;
	for (Item const* item : m_items)
	{
		weight += item->GetValueAsFloat("weight");
	}

	return weight;
}


bool Inventory::CanAddItemWithWeight(float weight)
{
	return weight + GetCurrentWeight() <= m_maxWeight;
}


void Inventory::AddItemToInventory(Item* item)
{
	m_items.push_back(item);
}


Item* Inventory::RemoveItemFromInventory(int index)
{
	if (index < 0 || index >= (int)m_items.size()) return nullptr;
	Item* item = m_items[index];
	m_items.erase(m_items.begin() + index);
	return item;
}


Item const* Inventory::GetItemByIndex(int index) const
{
	if (index < 0 || index >= (int)m_items.size()) return nullptr;
	return m_items[index];
}


int Inventory::GetIndexByName(std::string const& name)
{
	for (int index = 0; index < (int)m_items.size(); index++)
	{
		Item*& item = m_items[index];
		if (item->GetValueAsString("name") == name)
		{
			return index;
		}
	}

	return -1;
}


std::vector<Item*> Inventory::SortBy(std::string const& category, std::string const& itemKey, bool isDescend)
{
	if (category.empty())
	{
		if (itemKey.empty())
		{
			Filter::SortBy(m_items, Filter::GetSorterByName("name"), isDescend);
			return m_items;
		}
		else
		{
			Filter::SortBy(m_items, Filter::GetSorterByName(itemKey), isDescend);
			return m_items;
		}
	}
	else
	{
		std::vector<Item*> items;
		for (Item*& item : m_items)
		{
			if (item->GetValueAsString("category") == category)
			{
				items.push_back(item);
			}
		}

		if (itemKey.empty())
		{
			Filter::SortBy(items, Filter::GetSorterByName("name"), isDescend);
			return items;
		}
		else
		{
			Filter::SortBy(items, Filter::GetSorterByName(itemKey), isDescend);
			return items;
		}
	}
}


