#pragma once

#include <vector>
#include <functional>
#include <map>
#include <string>

struct Item;

class Filter
{
public:
	static void InitializeFilter();
	static void SortBy(std::vector<Item*>& items, std::function<bool(Item const* a, Item const* b)> sorter, bool isDescend = false);
	static std::function<bool(Item const* a, Item const* b)> GetSorterByName(std::string const& name);

public:
	static std::map<std::string, std::function<bool(Item const* a, Item const* b)>> s_sorters;
};